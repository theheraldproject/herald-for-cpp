//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/data.h"
#include "herald/ble/ble_mac_address.h"
#include "herald/ble/filter/ble_advert_parser.h"

// nRF Connect SDK includes
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/addr.h>

// C++17 includes
#include <memory>
#include <vector>
#include <cstring>
#include <map>
#include <sstream>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::data;
using namespace herald::ble::filter;


struct AddrRef {
  const bt_addr_le_t* addr;
};

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
    BT_GAP_INIT_CONN_INT_MIN, BT_GAP_INIT_CONN_INT_MAX, 0, 400
  );
  /**
   * Why is this necessary? Traditional pointer-to-function cannot easily
   * and reliably be wrapped with std::function/bind/mem_fn. We also need
   * the Herald API to use subclasses for each platform, necessitating
   * some sort of static bridge. Not pretty, but works and allows us to
   * prevent nullptr problems
   */
  std::optional<std::shared_ptr<herald::zephyrinternal::Callbacks>> 
    concreteReceiverInstance;
  
  
  void scan_init(void)
  {
    // int err;

    // struct bt_scan_init_param scan_init = {
    //   .connect_if_match = 0, // no auto connect (handled by herald protocol coordinator)
    //   .scan_param = NULL,
    //   .conn_param = BT_LE_CONN_PARAM_DEFAULT
    // };

    // bt_scan_init(&scan_init);
    // bt_scan_cb_register(&scan_cb);

    /*
    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, herald_uuid);
    if (err) {
      printk("Scanning filters cannot be set (err %d)\n", err);

      return;
    }

    err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
    if (err) {
      printk("Filters cannot be turned on (err %d)\n", err);
    }
    */
  }

  
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
  
  static struct bt_conn_cb conn_callbacks = {
    .connected        = connected,
    .disconnected     = disconnected
  };

  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf) {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->scan_cb(addr,rssi,adv_type,buf);
    }
  }

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

  void connected(struct bt_conn *conn, uint8_t err) override;
  void disconnected(struct bt_conn *conn, uint8_t reason) override;
  
  void discovery_completed_cb(struct bt_gatt_dm *dm, void *context) override;
  void discovery_service_not_found_cb(struct bt_conn *conn, void *context) override;
  void discovery_error_found_cb(struct bt_conn *conn, int err, void *context) override;

  // internal call methods
  void gatt_discover(struct bt_conn *conn);
      
  std::shared_ptr<ZephyrContext> m_context;
  std::shared_ptr<BluetoothStateManager> m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  std::shared_ptr<BLEDatabase> m_db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  std::map<TargetIdentifier,AddrRef> macs; // TODO remove items over time

  HLOGGER;
};

ConcreteBLEReceiver::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
  std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, 
  std::shared_ptr<BLEDatabase> bleDatabase)
  : m_context(std::static_pointer_cast<ZephyrContext>(ctx)), // Herald API guarantees this to be safe
    m_stateManager(bluetoothStateManager),
    m_pds(payloadDataSupplier),
    m_db(bleDatabase),
    delegates(),
    macs()
    HLOGGERINIT(ctx,"Sensor","BLE.ConcreteBLEReceiver")
{
  ;
}

ConcreteBLEReceiver::Impl::~Impl()
{
  ;
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
  std::string addrStr(addr_str);
  
  // HTDBG(addrStr);
  //HTDBG("taskConnectScanResults, didDiscover (device=%s)", (char*)addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  TargetIdentifier target((Data)bleMacAddress);
  macs.emplace(target,AddrRef{.addr = addr});
  // HTDBG((std::string)bleMacAddress);

  auto device = m_db->device(target);
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
    // auto device = mImpl->m_db->device(bleMacAddress); // For most devices this will suffice

    // TODO check for public herald service in ADV_IND packet - shown if a wearable or beacon in zephyr

    if (0 != heraldDataSegments.size()) {
      HTDBG("Found Herald Android pseudo device address");
      device->pseudoDeviceAddress(BLEMacAddress(heraldDataSegments.front())); // For devices with unnatural (very fast) ble mac rotation, we need to use this rotating data area (some Android devices)
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
        HTDBG("Unknown non Herald device - ignoring");
        HTDBG((std::string)bleMacAddress);
        device->ignore(true);
      }
    }
  }
}

void
ConcreteBLEReceiver::Impl::gatt_discover(struct bt_conn *conn)
{
  HTDBG("Attempting GATT service discovery");
  int err;

  err = bt_gatt_dm_start(conn, &zephyrinternal::herald_uuid.uuid, &zephyrinternal::discovery_cb, NULL);
  if (err) {
    HTDBG("could not start the discovery procedure, error code")
    HTDBG(std::to_string(err));
    return;
  }
  HTDBG("Service discovery succeeded... now do something with it in the callback!");
}


void
ConcreteBLEReceiver::Impl::connected(struct bt_conn *conn, uint8_t err)
{
  HTDBG("Zephyr connection callback. Mac of connected:");

  auto addr = bt_conn_get_dst(conn);
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  std::string addrStr(addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  HTDBG((std::string)bleMacAddress);

  if (err) {
    HTDBG("Connected: Error value:-");
    HTDBG(std::to_string(err));
  } else {
    // TODO log last connected time in BLE database
    gatt_discover(conn);
  }
}

void
ConcreteBLEReceiver::Impl::disconnected(struct bt_conn *conn, uint8_t reason)
{
  HTDBG("Zephyr disconnection callback. Mac of disconnected:");

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
}

// Discovery callbacks

void
ConcreteBLEReceiver::Impl::discovery_completed_cb(struct bt_gatt_dm *dm,
				   void *context)
{
	HTDBG("The GATT discovery procedure succeeded");
	int err;

	bt_gatt_dm_data_print(dm);

	// err = bt_hogp_handles_assign(dm, &hogp);
	// if (err) {
	// 	HTDBG("Could not init HIDS client object, error: ")
  //   HTDBG(std::to_string(err);
	// }

	err = bt_gatt_dm_data_release(dm);
	if (err) {
		HTDBG("Could not release the discovery data, error code: ");
    HTDBG(std::to_string(err));
	}
}

void
ConcreteBLEReceiver::Impl::discovery_service_not_found_cb(struct bt_conn *conn,
					   void *context)
{
	HTDBG("The service could not be found during the discovery");
}

void
ConcreteBLEReceiver::Impl::discovery_error_found_cb(struct bt_conn *conn,
				     int err,
				     void *context)
{
	HTDBG("The discovery procedure failed with ");
  HTDBG(std::to_string(err));
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
  
	bt_conn_cb_register(&zephyrinternal::conn_callbacks);

  // Ensure our zephyr context has bluetooth ready
  int startOk = mImpl->m_context->startBluetooth();
  if (0 != startOk) {
    HDBG("ERROR starting context bluetooth:-");
    HDBG(std::to_string(startOk));
  }
  struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE, // passive scan
		.options    = BT_LE_SCAN_OPT_NONE, // Scans for EVERYTHING
		.interval   = 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
		.window     = 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
	};

  // now start scanning and register callback
  // using namespace std::placeholders;
  // auto fcb = std::function<void(const bt_addr_le_t *addr, std::int8_t rssi, 
  //   std::uint8_t adv_type, struct net_buf_simple *buf)>(
  //     std::bind(&ConcreteBLEReceiver::Impl::scan_cb,mImpl.get(),_1,_2,_3,_4)
  // );
  // bt_le_scan_cb_t* ptrfcb = fcb;
  zephyrinternal::scan_init();
  int err = bt_le_scan_start(&scan_param, &zephyrinternal::scan_cb); // scan_cb linked via BT_SCAN_CB_INIT call
	if (0 != err) {
		HDBG("Starting scanning failed");
		return;
	}
  HDBG("ConcreteBLEReceiver::start completed successfully");
}

void
ConcreteBLEReceiver::stop()
{
  HDBG("ConcreteBLEReceiver::stop");
  
  herald::ble::zephyrinternal::concreteReceiverInstance.reset(); // destroys the shared_ptr not necessarily the underlying value

  // TODO now stop scanning
  int err = bt_le_scan_stop();
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

bool
ConcreteBLEReceiver::openConnection(const TargetIdentifier& toTarget)
{
  HDBG("openConnection called for");
  HDBG((std::string)toTarget);
  // fetch device bluetooth ID
  auto found = mImpl->macs.find(toTarget);
  if (found == mImpl->macs.end()) {
    HDBG("MAC not found");
    return false;
  }
  
  
  // temporarily stop scan - WORKAROUND for https://github.com/zephyrproject-rtos/zephyr/issues/20660
  bt_le_scan_stop();



  // attempt connection
  bt_conn* conn = NULL;
  // use bt_conn_lookup_addr_le first to see if we're already connected
  conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT,found->second.addr);
  bool ok = true;
  if (NULL == conn) {
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
    int success = bt_conn_le_create(found->second.addr,
      // zephyrinternal::BTLECreateParam,
      // zephyrinternal::BTLEConnParam,
      // BT_LE_CONN_PARAM_DEFAULT,
      // BT_CONN_LE_CREATE_CONN,
      &zephyrinternal::defaultCreateParam,
      &zephyrinternal::defaultConnParam,
      //BT_CONN_LE_CREATE_PARAM_INIT(BT_CONN_LE_OPT_NONE,BT_GAP_SCAN_FAST_INTERVAL,BT_GAP_SCAN_FAST_INTERVAL),
      //BT_LE_CONN_PARAM_INIT(BT_GAP_INIT_CONN_INT_MIN,BT_GAP_INIT_CONN_INT_MAX,0,400),
      &conn);
    HDBG(" - post connection attempt");
    if (0 != success) {
      HDBG(" - Issue connecting");
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
      // DONT DO THIS HERE - MANY REASONS IT CAN FAIL auto device = mImpl->m_db->device(toTarget);
      // HDBG(" - Ignoring following target: {}", toTarget);
      // device->ignore(true);
    }
  } else {
    HDBG(" - Existing connection exists! Reusing.");
  }
  if (ok) {
    HDBG("Connected - discovering GATT");
    // Perform GATT service discovery
    mImpl->gatt_discover(conn);
  }
  
  // Check return value
  // HDBG(" - returning from openConnection");
  return ok;
}

bool
ConcreteBLEReceiver::closeConnection(const TargetIdentifier& toTarget)
{
  return false;
}

void
ConcreteBLEReceiver::identifyOS(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::readPayload(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::immediateSend(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::immediateSendAll(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

}
}
