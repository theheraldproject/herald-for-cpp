//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/ble_sensor_configuration.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/data.h"
#include "herald/datatype/payload_data.h"
#include "herald/datatype/immediate_send_data.h"
#include "herald/ble/ble_mac_address.h"
#include "herald/ble/filter/ble_advert_parser.h"

// nRF Connect SDK includes
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/addr.h>
#include <bluetooth/scan.h>
#include <kernel.h>
#include <zephyr/types.h>
#include <zephyr.h>

// C++17 includes
#include <memory>
#include <vector>
#include <cstring>
#include <map>
#include <sstream>
// #include <mutex>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::data;
using namespace herald::ble::filter;

// ZEPHYR UTILITY FUNCTIONS
/** wait with timeout for Zephyr. Returns true if the function timed out rather than completed **/
uint32_t waitWithTimeout(uint32_t timeoutMillis, k_timeout_t period, std::function<bool()> keepWaiting)
{
  uint32_t start_time;
  uint32_t stop_time;
  uint32_t millis_spent;
  bool notComplete = keepWaiting();
  if (!notComplete) {
    return 0;
  }
  /* capture initial time stamp */
  start_time = k_uptime_get_32();

  /* capture final time stamp */
  stop_time = k_uptime_get_32();
  /* compute how long the work took (assumes no counter rollover) */
  millis_spent = stop_time - start_time;

  while (millis_spent < timeoutMillis && notComplete) {
    k_sleep(period);
    notComplete = keepWaiting();

    /* capture final time stamp */
    stop_time = k_uptime_get_32();
    /* compute how long the work took (assumes no counter rollover) */
    millis_spent = stop_time - start_time;
  }
  if (notComplete) {
    return millis_spent;
  } else {
    return 0;
  }
}


struct ConnectedDeviceState {
  ConnectedDeviceState(const TargetIdentifier& id)
    : target(id), state(BLEDeviceState::disconnected), connection(NULL), address(),
      readPayloadData(), immediateSendData()
  {}
  ConnectedDeviceState(const ConnectedDeviceState& from) = delete;
  ConnectedDeviceState(ConnectedDeviceState&& from) = delete;
  ~ConnectedDeviceState() = default;

  TargetIdentifier target;
  BLEDeviceState state;
  bt_conn* connection;
  bt_addr_le_t address;
  PayloadData readPayloadData;
  ImmediateSendData immediateSendData;
};


// struct AddrRef {
//   const bt_addr_le_t* addr;
// };

namespace zephyrinternal {
  
  /* Herald Service Variables */
  static struct bt_uuid_128 herald_uuid = BT_UUID_INIT_128(
    0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
  );
  static struct bt_uuid_128 herald_char_signal_uuid = BT_UUID_INIT_128(
    0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
  );
  static struct bt_uuid_128 herald_char_payload_uuid = BT_UUID_INIT_128(
    0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
  );
  

  bt_le_conn_param* BTLEConnParam = BT_LE_CONN_PARAM_DEFAULT; // BT_LE_CONN_PARAM(0x018,3200,0,400); // NOT BT_LE_CONN_PARAM_DEFAULT;
  bt_conn_le_create_param* BTLECreateParam = BT_CONN_LE_CREATE_CONN; // BT_CONN_LE_CREATE_PARAM(BT_CONN_LE_OPT_NONE, 0x0010,0x0010);// NOT BT_CONN_LE_CREATE_CONN;

  static struct bt_conn_le_create_param defaultCreateParam = BT_CONN_LE_CREATE_PARAM_INIT(
    BT_CONN_LE_OPT_NONE, BT_GAP_SCAN_FAST_INTERVAL, BT_GAP_SCAN_FAST_INTERVAL
  );
  static struct bt_le_conn_param defaultConnParam = BT_LE_CONN_PARAM_INIT(
    //BT_GAP_INIT_CONN_INT_MIN, BT_GAP_INIT_CONN_INT_MAX, 0, 400
    //12, 12 // aka 15ms, default from apple documentation
    0x50, 0x50, // aka 80ms, from nRF SDK LLPM sample
    0, 400
  );
  // Note for apple see: https://developer.apple.com/library/archive/qa/qa1931/_index.html
  // And https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf (BLE section)

  static struct bt_le_scan_param defaultScanParam = //BT_LE_SCAN_PASSIVE;
  {
		.type       = BT_LE_SCAN_TYPE_PASSIVE, // passive scan
		.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE, // Scans for EVERYTHING
		.interval   = BT_GAP_SCAN_FAST_INTERVAL, // 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
		.window     = BT_GAP_SCAN_FAST_WINDOW // 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
	};

  /**
   * Why is this necessary? Traditional pointer-to-function cannot easily
   * and reliably be wrapped with std::function/bind/mem_fn. We also need
   * the Herald API to use subclasses for each platform, necessitating
   * some sort of static bridge. Not pretty, but works and allows us to
   * prevent nullptr problems
   */
  std::optional<std::shared_ptr<herald::zephyrinternal::Callbacks>> 
    concreteReceiverInstance;
  
  
  // static struct bt_conn* conn = NULL;
  
  // NOTE: The below is called multiple times for ONE char value. Keep appending to result until NULL==data.
  static uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length)
  {
    if (concreteReceiverInstance.has_value()) {
      return concreteReceiverInstance.value()->gatt_read_cb(conn,err,params,data,length);
    }
    return length; // say we've consumed the data anyway
  }
  
  static struct bt_gatt_read_params read_params = {
    .func = gatt_read_cb,
    .handle_count = 1,
    .single = {
      .handle = 0x0000,
      .offset = 0x0000
    }
  };


  // void scan_init(void)
  // {
  //   // int err;

  //   // struct bt_scan_init_param scan_init = {
  //   //   .connect_if_match = 0, // no auto connect (handled by herald protocol coordinator)
  //   //   .scan_param = NULL,
  //   //   .conn_param = BT_LE_CONN_PARAM_DEFAULT
  //   // };

  //   // bt_scan_init(&scan_init);
  //   // bt_scan_cb_register(&scan_cb);

  //   /*
  //   err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, herald_uuid);
  //   if (err) {
  //     printk("Scanning filters cannot be set (err %d)\n", err);

  //     return;
  //   }

  //   err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
  //   if (err) {
  //     printk("Filters cannot be turned on (err %d)\n", err);
  //   }
  //   */
  // }

  
  static void connected(struct bt_conn *conn, uint8_t err)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->connected(conn,err);
    }
  }

  static void disconnected(struct bt_conn *conn, uint8_t reason)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->disconnected(conn,reason);
    }
  }

  static void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->le_param_updated(conn,interval,latency,timeout);
    }
  }
  
	static struct bt_conn_cb conn_callbacks = {
		.connected = connected,
		.disconnected = disconnected,
		.le_param_updated = le_param_updated,
	};

  // static bt_addr_le_t *last_addr = BT_ADDR_LE_NONE;

  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf) {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->scan_cb(addr,rssi,adv_type,buf);
    }
  }

  // BT_SCAN_CB_INIT(scan_cbs, scan_filter_match, );
  
	static struct bt_scan_init_param scan_init = {
		.scan_param = &defaultScanParam,
		.connect_if_match = false,
		.conn_param = &defaultConnParam
	};

  // void scan_filter_match(struct bt_scan_device_info *device_info,
  //             struct bt_scan_filter_match *filter_match,
  //             bool connectable)
  // {
  //   char addr[BT_ADDR_LE_STR_LEN];

  //   bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

  //   printk("Filters matched. Address: %s connectable: %s\n",
  //     addr, connectable ? "yes" : "no");
  // }

  // void scan_connecting_error(struct bt_scan_device_info *device_info)
  // {
  //   printk("Connecting failed\n");
  // }

  // void scan_connecting(struct bt_scan_device_info *device_info,
  //           struct bt_conn *conn)
  // {
  //   //default_conn = bt_conn_ref(conn);
  // }

  // void scan_filter_no_match(struct bt_scan_device_info *device_info,
  //         bool connectable)
  // {
  //   int err;
  //   struct bt_conn *conn;
  //   char addr[BT_ADDR_LE_STR_LEN];

  //   if (device_info->recv_info->adv_type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
  //     bt_addr_le_to_str(device_info->recv_info->addr, addr,
  //           sizeof(addr));
  //     printk("Direct advertising received from %s\n", addr);
  //     bt_scan_stop();

  //     err = bt_conn_le_create(device_info->recv_info->addr,
  //           BT_CONN_LE_CREATE_CONN,
  //           device_info->conn_param, &conn);

  //     if (!err) {
  //       default_conn = bt_conn_ref(conn);
  //       bt_conn_unref(conn);
  //     }
  //   }
  // }

  // BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_no_match,
  //     scan_connecting_error, scan_connecting);



  // GATT DISCOVERY INTERNAL METHODS
  
  static void discovery_completed_cb(struct bt_gatt_dm *dm,
            void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->discovery_completed_cb(dm,context);
    }
  }

  static void discovery_service_not_found_cb(struct bt_conn *conn,
              void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->discovery_service_not_found_cb(conn,context);
    }
  }

  static void discovery_error_found_cb(struct bt_conn *conn,
              int err,
              void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->discovery_error_found_cb(conn,err,context);
    }
  }

  static const struct bt_gatt_dm_cb discovery_cb = {
    .completed = discovery_completed_cb,
    .service_not_found = discovery_service_not_found_cb,
    .error_found = discovery_error_found_cb,
  };

}


class ConcreteBLEReceiver::Impl : public herald::zephyrinternal::Callbacks {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, 
    std::shared_ptr<BLEDatabase> bleDatabase);
  ~Impl();

  // Zephyr OS callbacks
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
      struct net_buf_simple *buf) override;

  void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) override;
  void connected(struct bt_conn *conn, uint8_t err) override;
  void disconnected(struct bt_conn *conn, uint8_t reason) override;
  
  void discovery_completed_cb(struct bt_gatt_dm *dm, void *context) override;
  void discovery_service_not_found_cb(struct bt_conn *conn, void *context) override;
  void discovery_error_found_cb(struct bt_conn *conn, int err, void *context) override;

  uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length) override;

  // std::optional<ConnectedDeviceState&> findState(const TargetIdentifier& forTarget);
  // std::optional<ConnectedDeviceState&> findStateByConnection(struct bt_conn *conn);
  ConnectedDeviceState& findOrCreateState(const TargetIdentifier& toTarget);
  ConnectedDeviceState& findOrCreateStateByConnection(struct bt_conn *conn);
  void removeState(const TargetIdentifier& forTarget);

  // internal call methods
  void startScanning();
  void gatt_discover(struct bt_conn *conn);
      
  std::shared_ptr<ZephyrContext> m_context;
  std::shared_ptr<BluetoothStateManager> m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  std::shared_ptr<BLEDatabase> db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  // std::map<TargetIdentifier,AddrRef> macs; // TODO remove items over time
  Data readPayloadData; // TODO move this to connectedDeviceState

  // std::optional<TargetIdentifier> targetForConnection;
  // std::optional<HeraldConnectionCallback> connCallback;

  // std::mutex bleInUse; // for ALL ble operations requiring a lock (connect, write, close, gatt, etc.)
  // std::condition_variable connectionAvailable;
  // DONT DO THIS - failure returns a conn too use zephyrinternal::conn for the condition checking routine
  // BLEDeviceState connectionState;

  // std::optional<TargetIdentifier> currentTarget;

  std::map<TargetIdentifier,ConnectedDeviceState> connectionStates;

  HLOGGER;
};

ConcreteBLEReceiver::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
  std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, 
  std::shared_ptr<BLEDatabase> bleDatabase)
  : m_context(std::static_pointer_cast<ZephyrContext>(ctx)), // Herald API guarantees this to be safe
    m_stateManager(bluetoothStateManager),
    m_pds(payloadDataSupplier),
    db(bleDatabase),
    delegates(),
    // macs(),
    readPayloadData(),
    // targetForConnection(),
    // connCallback(),
    // bleInUse(),
    // connectionAvailable(),
    // connectionState(BLEDeviceState::disconnected),
    // currentTarget(),
    connectionStates()
    HLOGGERINIT(ctx,"Sensor","BLE.ConcreteBLEReceiver")
{
  ;
}

ConcreteBLEReceiver::Impl::~Impl()
{
  ;
}

// NOTE: Optional references currently illegal in C++17 (Would need Boost)

// std::optional<ConnectedDeviceState&>
// ConcreteBLEReceiver::Impl::findState(const TargetIdentifier& forTarget)
// {
//   auto iter = connectionStates.find(forTarget);
//   if (connectionStates.end() != iter) {
//     return iter->second;
//   }
//   return {};
// }

// std::optional<ConnectedDeviceState&>
// ConcreteBLEReceiver::Impl::findStateByConnection(struct bt_conn *conn)
// {
//   for (const auto& [key, value] : connectionStates) {
//     if (value.connection == conn) {
//       return value;
//     }
//   }
//   return {};
// }

ConnectedDeviceState&
ConcreteBLEReceiver::Impl::findOrCreateState(const TargetIdentifier& forTarget)
{
  auto iter = connectionStates.find(forTarget);
  if (connectionStates.end() != iter) {
    return iter->second;
  }
  return connectionStates.emplace(forTarget, forTarget).first->second;
  // return connectionStates.find(forTarget)->second;
}

ConnectedDeviceState&
ConcreteBLEReceiver::Impl::findOrCreateStateByConnection(struct bt_conn *conn)
{
  for (auto& [key, value] : connectionStates) {
    if (value.connection == conn) {
      return value;
    }
  }
  // Create target identifier from address
  auto addr = bt_conn_get_dst(conn);
  BLEMacAddress bleMacAddress(addr->a.val);
  TargetIdentifier target((Data)bleMacAddress);
  auto result = connectionStates.emplace(target, target);
  bt_addr_le_copy(&result.first->second.address,addr);
  return result.first->second;
}

void
ConcreteBLEReceiver::Impl::removeState(const TargetIdentifier& forTarget)
{
  auto iter = connectionStates.find(forTarget);
  if (connectionStates.end() != iter) {
    connectionStates.erase(iter);
  }
}

void
ConcreteBLEReceiver::Impl::startScanning()
{
  int err = bt_le_scan_start(&zephyrinternal::defaultScanParam, &zephyrinternal::scan_cb); // scan_cb linked via BT_SCAN_CB_INIT call
  
  //int err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE); // calls bt_le_scan_start
  HTDBG("back from bt scan start");
  if (0 != err) {
		HTDBG("Starting scanning failed");
		return;
	}
}
  
void
ConcreteBLEReceiver::Impl::scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf)
{
  // HTDBG("Callback");
  // Data advert;
  // // Convert advert scan data to Herald format
  // for (std::uint16_t p = 0;p < buf->len;p++) {
  //   advert.append(std::uint8_t(buf->data[p]));
  // }
  
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  // zephyrinternal::last_addr = addr;
  // bt_addr_le_copy(zephyrinternal::last_addr,addr);
  std::string addrStr(addr_str);
  HTDBG("ADDR FROM SCAN:-");
  HTDBG(addr_str);
  
  // HTDBG(addrStr);
  //HTDBG("taskConnectScanResults, didDiscover (device=%s)", (char*)addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  TargetIdentifier target((Data)bleMacAddress);
  // macs.emplace(target,AddrRef{.addr = addr});
  // HTDBG((std::string)bleMacAddress);

  auto device = db->device(target);
  // HTDBG("taskConnectScanResults, didDiscover (deviceMAC={})", (std::string)bleMacAddress);

  // // Now pass to relevant BLEDatabase API call
  if (!device->rssi().has_value()) {
    device->rssi(RSSI(rssi));
    Data advert(buf->data,buf->len);
    auto segments = BLEAdvertParser::extractSegments(advert,0);
    // HTDBG("segments:-");
    // HTDBG(std::to_string(segments.size()));
    auto manuData = BLEAdvertParser::extractManufacturerData(segments);
    auto heraldDataSegments = BLEAdvertParser::extractHeraldManufacturerData(manuData);
    // HTDBG("herald data segments:-");
    // HTDBG(std::to_string(heraldDataSegments.size()));
    // auto device = mImpl->db->device(bleMacAddress); // For most devices this will suffice

    // TODO Check for public herald service in ADV_IND packet - shown if an Android device, wearable or beacon in zephyr
    // auto serviceData128 = BLEAdvertParser::extractServiceUUID128Data(segments);
    // bool hasHeraldService = false;
    // for (auto& service : serviceData128) {
    //   if (service.uuid == heraldUuidData) {
    //     hasHeraldService = true;
    //     HTDBG("FOUND DEVICE ADVERTISING HERALD SERVICE");
    //     device->operatingSystem(BLEDeviceOperatingSystem::android);
    //   }
    // }
    

    if (0 != heraldDataSegments.size()) {
      HTDBG("Found Herald Android pseudo device address");
      device->pseudoDeviceAddress(BLEMacAddress(heraldDataSegments.front())); // For devices with unnatural (very fast) ble mac rotation, we need to use this rotating data area (some Android devices)
      device->operatingSystem(BLEDeviceOperatingSystem::android);
    } else {
      // If it's an apple device, check to see if its on our ignore list
      auto appleDataSegments = BLEAdvertParser::extractAppleManufacturerSegments(manuData);
      if (0 != appleDataSegments.size()) {
        HTDBG("Found apple device");
        HTDBG((std::string)bleMacAddress);
        device->operatingSystem(BLEDeviceOperatingSystem::ios);
        // TODO see if we should ignore this Apple device
        // TODO abstract these out eventually
        bool ignore = false;
        /*
"^10....04",
            "^10....14",
            "^0100000000000000000000000000000000",
            "^05","^07","^09",
            "^00","^1002","^06","^08","^03","^0C","^0D","^0F","^0E","^0B"
        */
        for (auto& segment : appleDataSegments) {
          HTDBG(segment.data.hexEncodedString());
          switch (segment.type) {
            case 0x00:
            case 0x05:
            case 0x07:
            case 0x09:
            case 0x06:
            case 0x08:
            case 0x03:
            case 0x0C:
            case 0x0D:
            case 0x0F:
            case 0x0E:
            case 0x0B:
              ignore = true;
              break;
            case 0x10:
              // check if second is 02
              if (segment.data.at(0) == std::byte(0x02)) {
                ignore = true;
              } else {
                // Check 3rd data bit for 14 or 04
                if (segment.data.at(2) == std::byte(0x04) || segment.data.at(2) == std::byte(0x14)) {
                  ignore = true;
                }
              }
              break;
            default:
              break;
          }
        }
        if (ignore) {
          HTDBG(" - Ignoring Apple device due to Apple data filter");
          device->ignore(true);
        } else {
          // Perform GATT service discovery to check for Herald service
          // NOTE: Happens from Connection request (handled by BLE Coordinator)
        }
      } else {
        // Not a Herald android or any iOS - so Ignore
        HTDBG("Unknown non Herald device - inspecting");
        HTDBG((std::string)bleMacAddress);
        // device->ignore(true);
      }
    }
  }
}

void
ConcreteBLEReceiver::Impl::gatt_discover(struct bt_conn *conn)
{
  HTDBG("Attempting GATT service discovery");
  int err;

  // Reinitialise data
  readPayloadData = Data();

  // begin introspection
  err = bt_gatt_dm_start(conn, &zephyrinternal::herald_uuid.uuid, &zephyrinternal::discovery_cb, NULL);
  if (err) {
    HTDBG("could not start the discovery procedure, error code")
    HTDBG(std::to_string(err));
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN); // ensures disconnect() called, and loop completed
    return;
  }
  HTDBG("Service discovery succeeded... now do something with it in the callback!");
}

void
ConcreteBLEReceiver::Impl::le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout)
{
  HTDBG("le param updated called");
}

void
ConcreteBLEReceiver::Impl::connected(struct bt_conn *conn, uint8_t err)
{
  HTDBG("**************** Zephyr connection callback. Mac of connected:");
	// if (conn != zephyrinternal::conn) {
  //   HTDBG("  - WARNING connected callback conn is not the same as the global conn - timing issue?");
	// 	return;
	// }

  auto addr = bt_conn_get_dst(conn);
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  std::string addrStr(addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  HTDBG((std::string)bleMacAddress);

  ConnectedDeviceState& state = findOrCreateStateByConnection(conn);

  if (err) {
    HTDBG("Connected: Error value:-");
    HTDBG(std::to_string(err));

		bt_conn_unref(conn);
    
    state.state = BLEDeviceState::disconnected;
    state.connection = NULL;
    // state.address = NULL;

		// startScanning();
    // if (targetForConnection.has_value() && connCallback.has_value()) {
    //   connCallback.value()(targetForConnection.value(),false);
    // }
		return;
  }

  // TODO log last connected time in BLE database

  state.connection = conn;
  // state.address = BT_ADDR_LE_NONE;
  bt_addr_le_copy(&state.address,addr);
  state.state = BLEDeviceState::connected;

  
  // if (targetForConnection.has_value() && connCallback.has_value()) {
  //   connCallback.value()(targetForConnection.value(),true);
  // }

}

void
ConcreteBLEReceiver::Impl::disconnected(struct bt_conn *conn, uint8_t reason)
{
  HTDBG("********** Zephyr disconnection callback. Mac of disconnected:");

  auto addr = bt_conn_get_dst(conn);
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  std::string addrStr(addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  HTDBG((std::string)bleMacAddress);

  if (reason) {
    HTDBG("Disconnection: Reason value:-");
    HTDBG(std::to_string(reason));
  }

  // TODO log disconnection time in ble database
  
	bt_conn_unref(conn);
  ConnectedDeviceState& state = findOrCreateStateByConnection(conn);

  state.state = BLEDeviceState::disconnected;
  state.connection = NULL;
  // state.address = NULL;

	// startScanning();
}

// Discovery callbacks

void
ConcreteBLEReceiver::Impl::discovery_completed_cb(struct bt_gatt_dm *dm,
				   void *context)
{
	HTDBG("The GATT discovery procedure succeeded");
  const struct bt_gatt_dm_attr *prev = NULL;
  bool found = false;
  do {
    prev = bt_gatt_dm_char_next(dm,prev);
    if (NULL != prev) {
      // Check for match of uuid to a herald read payload char
      struct bt_gatt_chrc *chrc = bt_gatt_dm_attr_chrc_val(prev);
      //if (chrc->uuid->type != BT_UUID_TYPE_128) continue; - not needed, done in cmp

      int matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::herald_char_payload_uuid.uuid);
      if (0 == matches) {
        HTDBG("    - FOUND Herald read characteristic. Reading.");
        // if match, for a read
        found = true;
        // set handles



        // TODO REFACTOR THE ACTUAL FETCHING OF PAYLOAD TO READPAYLOAD FUNCTION
        zephyrinternal::read_params.single.handle = chrc->value_handle;
        zephyrinternal::read_params.single.offset = 0x0000; // gets changed on each use
        int readErr = bt_gatt_read(bt_gatt_dm_conn_get(dm), &zephyrinternal::read_params);
        if (readErr) {
          HTDBG("GATT read error: TBD");//, readErr);
          // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        }



        break; // leave do while loop once found
      } else {
        char uuid_str[32];
        bt_uuid_to_str(chrc->uuid,uuid_str,sizeof(uuid_str));
        HTDBG("    - Char doesn't match read payload: TBD"); //, log_strdup(uuid_str));
        HTDBG(uuid_str);
      }
    }
  } while (NULL != prev);

  if (found) {
  } else {
    HTDBG("Herald read payload char not found in herald service (weird...)");
    // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
  }

  // err = bt_hogp_handles_assign(dm, &hogp);
  // if (err) {
  // 	HTDBG("Could not init HIDS client object, error: ")
//   HTDBG(std::to_string(err);
  // }


  ConnectedDeviceState& state = findOrCreateStateByConnection(bt_gatt_dm_conn_get(dm));

  auto device = db->device(state.target);
  std::vector<UUID> serviceList;
  serviceList.push_back(BLESensorConfiguration::serviceUUID);
  device->services(serviceList);

  // No it doesn't - this is safe: does ending this here break our bt_gatt_read? (as it uses that connection?)
  int err = bt_gatt_dm_data_release(dm);
  if (err) {
    HTDBG("Could not release the discovery data, error code: TBD");
    // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
  }

  // TODO replace all disconnect calls above with callback to fire didDiscoverCharacteristics callback
}

void
ConcreteBLEReceiver::Impl::discovery_service_not_found_cb(struct bt_conn *conn,
					   void *context)
{
	HTDBG("The service could not be found during the discovery. Ignoring device");
  ConnectedDeviceState& state = findOrCreateStateByConnection(conn);

  auto device = db->device(state.target);
  std::vector<UUID> serviceList; // empty service list // TODO put other listened-for services here
  device->services(serviceList);
  device->ignore(true);
  
  // bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void
ConcreteBLEReceiver::Impl::discovery_error_found_cb(struct bt_conn *conn,
				     int err,
				     void *context)
{
	HTDBG("The discovery procedure failed with ");
  HTDBG(std::to_string(err));
  // TODO decide if we should ignore the device here, or just keep trying
}

uint8_t
ConcreteBLEReceiver::Impl::gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length)
  {
    // LOG_INF("GATT READ CB CALLED");
    if (NULL == data) {
      // LOG_INF("Finished gatt value read");
      HTDBG("Finished reading CHAR read payload:-");
      HTDBG(readPayloadData.hexEncodedString());
      // Now disconnect post payload read
      bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
      // NOTE OR read next characteristic value, as required
      return 0;
    }
    // char hex_str[length * 2];
    // int transferred = bin2hex(data,length,hex_str,length * 2);
    // LOG_INF("  hex: %s",log_strdup(hex_str)); // do we need to append \0?
    readPayloadData.append((const uint8_t*)data,0,length);
    return length;
  }

// FIGURE OUT WHY INCLUDING BLE RECEIVER CPP FILE EVEN IF UNUSED STOPS THE DEVICE FUNCTIONING AND LOADING USB DRIVE

// 2. Implement discovery callbacks and test

// 4. Implement all other connect/disconnect logic and BLE device lifecycle methods

// 5. Implement Apple device pro-active filtering








ConcreteBLEReceiver::ConcreteBLEReceiver(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
  std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : mImpl(std::make_shared<Impl>(ctx,bluetoothStateManager,payloadDataSupplier,bleDatabase))
{
  ;
}

ConcreteBLEReceiver::~ConcreteBLEReceiver()
{
  ;
}

std::optional<std::shared_ptr<CoordinationProvider>>
ConcreteBLEReceiver::coordinationProvider()
{
  return {}; // we don't provide this, ConcreteBLESensor provides this. We provide HeraldV1ProtocolProvider
}

void
ConcreteBLEReceiver::add(std::shared_ptr<SensorDelegate> delegate)
{
  mImpl->delegates.push_back(delegate);
}

void
ConcreteBLEReceiver::start()
{
  HDBG("ConcreteBLEReceiver::start");
  herald::ble::zephyrinternal::concreteReceiverInstance = mImpl;
  

  // Ensure our zephyr context has bluetooth ready
  HDBG("calling start bluetooth");
  int startOk = mImpl->m_context->startBluetooth();
  HDBG("start bluetooth done");
  if (0 != startOk) {
    HDBG("ERROR starting context bluetooth:-");
    HDBG(std::to_string(startOk));
  }
  // struct bt_le_scan_param scan_param = {
	// 	.type       = BT_LE_SCAN_TYPE_PASSIVE, // passive scan
	// 	.options    = BT_LE_SCAN_OPT_NONE, // Scans for EVERYTHING
	// 	.interval   = 0x0080, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
	// 	.window     = 0x0080, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
	// };

  // now start scanning and register callback
  // using namespace std::placeholders;
  // auto fcb = std::function<void(const bt_addr_le_t *addr, std::int8_t rssi, 
  //   std::uint8_t adv_type, struct net_buf_simple *buf)>(
  //     std::bind(&ConcreteBLEReceiver::Impl::scan_cb,mImpl.get(),_1,_2,_3,_4)
  // );
  // bt_le_scan_cb_t* ptrfcb = fcb;
  // zephyrinternal::scan_init();


  HDBG("Calling conn cb register");
	bt_conn_cb_register(&zephyrinternal::conn_callbacks);
  HDBG("conn cb register done");

  // HDBG("calling bt scan init");
  // bt_scan_init(&zephyrinternal::scan_init);
  // HDBG("back from bt scan init");

  // bt_scan_cb_register(&zephyrinternal::scan_cbs); // for filtering API only
  HDBG("calling bt scan start");
  
  
  mImpl->startScanning();

  HDBG("ConcreteBLEReceiver::start completed successfully");
}

void
ConcreteBLEReceiver::stop()
{
  HDBG("ConcreteBLEReceiver::stop");
  
  herald::ble::zephyrinternal::concreteReceiverInstance.reset(); // destroys the shared_ptr not necessarily the underlying value

  // now stop scanning
  int err = bt_le_scan_stop(); //bt_scan_stop();
  if (err) {
		// mImpl->logger.info("Stopping scanning failed (err {:d})", err);
  }

  // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
  
  HDBG("ConcreteBLEReceiver::stop completed successfully");
}


bool
ConcreteBLEReceiver::immediateSend(Data data, const TargetIdentifier& targetIdentifier)
{
  return false; // TODO implement this
}

bool
ConcreteBLEReceiver::immediateSendAll(Data data)
{
  return false; // TODO implement this
}


// Herald V1 Protocol Provider overrides

// void
// ConcreteBLEReceiver::openConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback)
// {
bool
ConcreteBLEReceiver::openConnection(const TargetIdentifier& toTarget)
{
  HDBG("openConnection");

  // Create addr from TargetIdentifier data
  ConnectedDeviceState& state = mImpl->findOrCreateState(toTarget);
  uint8_t val[6] = {0,0,0,0,0,0};
  Data addrData = (Data)toTarget; // TODO change this to mac for target ID
  uint8_t t;
  bool cok = addrData.uint8(0,t);
  if (cok)
  val[0] = t;
  cok = addrData.uint8(1,t);
  if (cok)
  val[1] = t;
  cok = addrData.uint8(2,t);
  if (cok)
  val[2] = t;
  cok = addrData.uint8(3,t);
  if (cok)
  val[3] = t;
  cok = addrData.uint8(4,t);
  if (cok)
  val[4] = t;
  cok = addrData.uint8(5,t);
  if (cok)
  val[5] = t;
  // TODO create a convenience function in Data for the above
  
  // TODO don't assume RANDOM (1) in the below
  bt_addr_le_t tempAddress{1, {{val[0],val[1],val[2],val[3],val[4],val[5]}}};
  // state.address = BT_ADDR_LE_NONE;
  bt_addr_le_copy(&state.address, &tempAddress);
  HDBG("Address copied. Constituted as:-");
  // idiot check of copied data
  Data newAddr(state.address.a.val,6);
  BLEMacAddress newMac(newAddr);
  HDBG((std::string)newMac);



  // HDBG("openConnection called for");
  // HDBG((std::string)toTarget);
  // fetch device bluetooth ID
  // auto found = mImpl->macs.find(toTarget);
  // if (found == mImpl->macs.end()) {
  //   HDBG("MAC not found");
  //   // connCallback(toTarget,false);
  //   return false;
  // }
  // mImpl->connCallback = connCallback;
  // mImpl->targetForConnection = toTarget;

  // print out device info
  BLEMacAddress mac(addrData);
  std::string di("Opening Connection :: Device info: mac=");
  di += (std::string)mac;
  di += ", os=";
  auto devPtr = mImpl->db->device(toTarget);
  auto os = devPtr->operatingSystem();
  if (os.has_value()) {
    if (herald::ble::BLEDeviceOperatingSystem::ios == os) {
      di += "ios";
    } else if (herald::ble::BLEDeviceOperatingSystem::android == os) {
      di += "android";
    }
  } else {
    di += "unknown";
  }
  di += ", ignore=";
  auto ignore = devPtr->ignore();
  if (ignore) {
    di += "true";
  } else {
    di += "false";
  }

  HDBG(di);
  
  
  // temporarily stop scan - WORKAROUND for https://github.com/zephyrproject-rtos/zephyr/issues/20660
  // HDBG("pausing scanning");
  bt_le_scan_stop();
  // HDBG("Scanning paused");
  // TODO restart after connect called


  // attempt connection
  // struct bt_conn* conn = NULL;
  // use bt_conn_lookup_addr_le first to see if we're already connected
  // HDBG("Looking up conn for this address");
  // conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT,found->second.addr);
  // HDBG("conn lookup returned");
  bool ok = true;
  if (NULL == state.connection) {
    HDBG(" - No existing connection. Attempting to connect");
    // std::stringstream os;
    // os << " - Create Param: Interval: " << zephyrinternal::defaultCreateParam.interval
    //    << ", Window: " << zephyrinternal::defaultCreateParam.window
    //    << ", Timeout: " << zephyrinternal::defaultCreateParam.timeout
    //    << " | Conn Param: Interval Min: " << zephyrinternal::BTLEConnParam->interval_min
    //    << ", Interval Max: " << zephyrinternal::defaultConnParam.interval_max
    //    << ", latency: " << zephyrinternal::defaultConnParam.latency
    //    << ", timeout: " << zephyrinternal::defaultConnParam.timeout
    //    << std::ends
    //    ;
    // HDBG(os.str());
    // HDBG(std::to_string(zephyrinternal::defaultCreateParam.interval));
    // HDBG(std::to_string(zephyrinternal::defaultCreateParam.window));
    // HDBG(std::to_string(zephyrinternal::defaultCreateParam.timeout));
    // HDBG(std::to_string(zephyrinternal::defaultConnParam.interval_min));
    // HDBG(std::to_string(zephyrinternal::defaultConnParam.interval_max));
    // HDBG(std::to_string(zephyrinternal::defaultConnParam.latency));
    // HDBG(std::to_string(zephyrinternal::defaultConnParam.timeout));
    // HDBG("Random address check ok?");
    // HDBG(bt_le_scan_random_addr_check() ? "yes" : "no");
    
  	char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&state.address, addr_str, sizeof(addr_str));
    HDBG("ADDR AS STRING in openConnection:-");
    HDBG(addr_str);
    // zephyrinternal::conn = NULL;
    state.state = BLEDeviceState::connecting; // this is used by the condition variable
    int success = bt_conn_le_create(
      //found->second.addr,
      // zephyrinternal::last_addr,
      &state.address,
      // zephyrinternal::BTLECreateParam,
      // zephyrinternal::BTLEConnParam,
      // BT_LE_CONN_PARAM_DEFAULT,
      // BT_CONN_LE_CREATE_CONN,
      // BT_CONN_LE_CREATE_CONN,
      // BT_LE_CONN_PARAM_DEFAULT
      &zephyrinternal::defaultCreateParam,
      &zephyrinternal::defaultConnParam,
      //BT_CONN_LE_CREATE_PARAM_INIT(BT_CONN_LE_OPT_NONE,BT_GAP_SCAN_FAST_INTERVAL,BT_GAP_SCAN_FAST_INTERVAL),
      //BT_LE_CONN_PARAM_INIT(BT_GAP_INIT_CONN_INT_MIN,BT_GAP_INIT_CONN_INT_MAX,0,400),
      // &zephyrinternal::conn
      &state.connection
      );
    HDBG(" - post connection attempt");
    if (0 != success) {
      // HDBG(" - Issue connecting");
      ok = false;
      if (-EINVAL == success) {
        HDBG(" - ERROR in passed in parameters");
      } else if (-EAGAIN == success) {
        HDBG(" - bt device not ready");
      } else if (-EALREADY == success) {
        HDBG(" - bt device initiating")
      } else if (-ENOMEM == success) {
        HDBG(" - bt connect attempt failed with default BT ID");
      } else if (-ENOBUFS == success) {
        HDBG(" - bt_hci_cmd_create has no buffers free");
      } else if (-ECONNREFUSED == success) {
        HDBG(" - Connection refused");
      } else if (-EIO == success) {
        HDBG(" - Low level BT HCI opcode IO failure");
      } else {
        HDBG(" - Unknown error code...");
        HDBG(std::to_string(success));
      }
      // HDBG(" - Issue connecting: {}",std::to_string(success));

      // Add to ignore list for now
      // DONT DO THIS HERE - MANY REASONS IT CAN FAIL auto device = mImpl->db->device(toTarget);
      // HDBG(" - Ignoring following target: {}", toTarget);
      // device->ignore(true);

      // restart scanning on failure
      // mImpl->startScanning();
      // connCallback(toTarget,false);
      return false;
    } else {
      HDBG("Zephyr waitWithTimeout for new connection");
      // lock and wait for connection to be created
      // STD::ASYNC/MUTEX variant:-
      // std::unique_lock<std::mutex> lk(mImpl->bleInUse);
      // mImpl->connectionAvailable.wait(lk, [this] {
      //   return mImpl->connectionState == BLEDeviceState::connecting;
      // }); // BLOCKS
      // verify connection successful
      // connCallback(toTarget,mImpl->connectionState == BLEDeviceState::connected);

      // ZEPHYR SPECIFIC VARIANT
      uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&state] {
        // return mImpl->connectionState == BLEDeviceState::connecting;
        return state.state == BLEDeviceState::connecting;
      });
      if (timedOut != 0) {
        HDBG("ZEPHYR WAIT TIMED OUT. Is connected?");
        // HDBG((mImpl->connectionState == BLEDeviceState::connected) ? "true" : "false");
        HDBG((state.state == BLEDeviceState::connected) ? "true" : "false");
        HDBG(std::to_string(timedOut));
        return false;
      }
      // return mImpl->connectionState == BLEDeviceState::connected;
      return state.state == BLEDeviceState::connected;
    }
  } else {
    HDBG(" - Existing connection exists! Reusing.");
    // zephyrinternal::conn = conn;
    // Assume discovery performed on first run through connect()
    // // Perform GATT service discovery
    // mImpl->gatt_discover(conn);
    // connCallback(toTarget,true);
    return true;
  }
}

// void
// ConcreteBLEReceiver::closeConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback)
// {
//   connCallback(toTarget,false);
// }

// void
// ConcreteBLEReceiver::serviceDiscovery(Activity activity, CompletionCallback callback)
// {
//   callback(activity,{});
// }

// void
// ConcreteBLEReceiver::readPayload(Activity activity, CompletionCallback callback)
// {
//   callback(activity,{});
// }

// void
// ConcreteBLEReceiver::immediateSend(Activity activity, CompletionCallback callback)
// {
//   callback(activity,{});
// }

// void
// ConcreteBLEReceiver::immediateSendAll(Activity activity, CompletionCallback callback)
// {
//   callback(activity,{});
// }

bool
ConcreteBLEReceiver::closeConnection(const TargetIdentifier& toTarget)
{
  HDBG("closeConnection call for ADDR:-");
  ConnectedDeviceState& state = mImpl->findOrCreateState(toTarget);
  char addr_str[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(&state.address, addr_str, sizeof(addr_str));
  HDBG(addr_str);
  if (NULL != state.connection) {
    // bt_conn_disconnect(zephyrinternal::conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    bt_conn_disconnect(state.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
  }
  mImpl->removeState(toTarget);
  mImpl->startScanning();
  return false; // assumes we've closed it // TODO proper multi-connection state tracking
}

std::optional<Activity>
ConcreteBLEReceiver::serviceDiscovery(Activity activity)
{
  auto currentTargetOpt = std::get<1>(activity.prerequisites.front());
  if (!currentTargetOpt.has_value()) {
    HDBG("No target specified for serviceDiscovery activity. Returning.");
    return {}; // We've been asked to connect to no specific target - not valid for Bluetooth
  }
  // Ensure we have a cached state (i.e. we are connected)
  auto& state = mImpl->findOrCreateState(currentTargetOpt.value());
  if (state.state != BLEDeviceState::connected) {
    HDBG("Not connected to target of activity. Returning.");
    return {};
  }
  if (NULL == state.connection) {
    HDBG("State for activity does not have a connection. Returning.")
    return {};
  }
  auto device = mImpl->db->device(currentTargetOpt.value());

  mImpl->gatt_discover(state.connection);

  uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&device] () -> bool {
    return !device->hasServicesSet(); // service discovery not completed yet
  });
  // mImpl->currentTarget = {};
  if (0 != timedOut) {
    HDBG("service discovery timed out for device");
    HDBG(std::to_string(timedOut));
    return {};
  }
  return {};
}

std::optional<Activity>
ConcreteBLEReceiver::readPayload(Activity activity)
{
  return {};
}

std::optional<Activity>
ConcreteBLEReceiver::immediateSend(Activity activity)
{
  return {};
}

std::optional<Activity>
ConcreteBLEReceiver::immediateSendAll(Activity activity)
{
  return {};
}

}
}
