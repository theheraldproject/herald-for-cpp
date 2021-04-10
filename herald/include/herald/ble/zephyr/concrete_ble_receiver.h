//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_RECEIVER_H
#define HERALD_BLE_CONCRETE_RECEIVER_H

#include "../ble_database.h"
#include "../ble_receiver.h"
#include "../ble_sensor.h"
#include "../ble_transmitter.h"
#include "../ble_protocols.h"
#include "../bluetooth_state_manager.h"
#include "../ble_device_delegate.h"
#include "../filter/ble_advert_parser.h"
#include "../../payload/payload_data_supplier.h"
#include "../../context.h"
#include "../../data/sensor_logger.h"
#include "../ble_sensor_configuration.h"
#include "../ble_coordinator.h"
#include "../../datatype/bluetooth_state.h"
#include "../ble_mac_address.h"
#include "../../zephyr_context.h"

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
#include <algorithm>
#include <optional>
#include <map>
#include <functional>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;


// ZEPHYR UTILITY FUNCTIONS
/** wait with timeout for Zephyr. Returns true if the function timed out rather than completed **/
uint32_t waitWithTimeout(uint32_t timeoutMillis, k_timeout_t period, std::function<bool()> keepWaiting);

struct ConnectedDeviceState {
  ConnectedDeviceState(const TargetIdentifier& id)
    : target(id), state(BLEDeviceState::disconnected), connection(NULL), address(),
      readPayload(), immediateSend(), remoteInstigated(false)
  {}
  ConnectedDeviceState(const ConnectedDeviceState& from) = delete;
  ConnectedDeviceState(ConnectedDeviceState&& from) = delete;
  ~ConnectedDeviceState() = default;

  TargetIdentifier target;
  BLEDeviceState state;
  bt_conn* connection;
  bt_addr_le_t address;
  PayloadData readPayload;
  ImmediateSendData immediateSend;
  bool remoteInstigated;
};

namespace zephyrinternal {
  
  struct bt_uuid_128* getHeraldUUID();
  struct bt_uuid_128* getHeraldSignalAndroidCharUUID();
  struct bt_uuid_128* getHeraldSignalIOSCharUUID();
  struct bt_uuid_128* getHeraldPayloadCharUUID();

  void setReceiverInstance(herald::zephyrinternal::Callbacks& cbref);
  void resetReceiverInstance();

  struct bt_conn_le_create_param* getDefaultCreateParam();
  struct bt_le_conn_param* getDefaultConnParam();

  struct bt_le_scan_param* getDefaultScanParam();
  struct bt_scan_init_param* getScanInitParam();

  struct bt_gatt_read_params* getReadParams();
  
  // static struct bt_conn* conn = NULL;
  
  [[maybe_unused]]
  // NOTE: The below is called multiple times for ONE char value. Keep appending to result until NULL==data.
  uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length);


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

  
  [[maybe_unused]]
  void connected(struct bt_conn *conn, uint8_t err);

  [[maybe_unused]]
  void disconnected(struct bt_conn *conn, uint8_t reason);

  [[maybe_unused]]
  void le_param_updated(struct bt_conn *conn, uint16_t interval,
    uint16_t latency, uint16_t timeout);

  struct bt_conn_cb* getConnectionCallbacks();

  // static bt_addr_le_t *last_addr = BT_ADDR_LE_NONE;

  [[maybe_unused]]
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
    struct net_buf_simple *buf);


  // BT_SCAN_CB_INIT(scan_cbs, scan_filter_match, );

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
  
  void discovery_completed_cb(struct bt_gatt_dm *dm,
    void *context);

  void discovery_service_not_found_cb(struct bt_conn *conn,
    void *context);

  void discovery_error_found_cb(struct bt_conn *conn,
    int err,
    void *context);

  const struct bt_gatt_dm_cb* getDiscoveryCallbacks();
}

template <typename ContextT, typename BLEDatabaseT>
class ConcreteBLEReceiver : public BLEReceiver, public HeraldProtocolV1Provider, public herald::zephyrinternal::Callbacks /*, public std::enable_shared_from_this<ConcreteBLEReceiver<ContextT>>*/ {
public:
  ConcreteBLEReceiver(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, BLEDatabaseT& bleDatabase)
    : m_context(ctx), // Herald API guarantees this to be safe
      m_stateManager(bluetoothStateManager),
      m_pds(payloadDataSupplier),
      db(bleDatabase),
      delegates(),
      connectionStates(),
      isScanning(false)
      HLOGGERINIT(ctx,"Sensor","BLE.ConcreteBLEReceiver")
  {
    ;
  }

  ConcreteBLEReceiver(const ConcreteBLEReceiver& from) = delete;
  ConcreteBLEReceiver(ConcreteBLEReceiver&& from) = delete;

  ~ConcreteBLEReceiver() {
    ;
  }

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() override
  {
    return {}; // we don't provide this, ConcreteBLESensor provides this. We provide HeraldV1ProtocolProvider
  }

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override
  {
    return false;
  }

  bool immediateSendAll(Data data) override {
    return false;
  }

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override
  {
    delegates.push_back(delegate);
  }

  void start() override
  {
    HTDBG("ConcreteBLEReceiver::start");
    if (!m_context.getSensorConfiguration().scanningEnabled) {
      HTDBG("Sensor Configuration has scanning disabled. Returning.");
      return;
    }
    herald::ble::zephyrinternal::setReceiverInstance(*this);
    

    // Ensure our zephyr context has bluetooth ready
    HTDBG("calling start bluetooth");
    int startOk = m_context.getPlatform().startBluetooth();
    HTDBG("start bluetooth done");
    if (0 != startOk) {
      HTDBG("ERROR starting context bluetooth:-");
      HTDBG(std::to_string(startOk));
    }

    HTDBG("Calling conn cb register");
    bt_conn_cb_register(zephyrinternal::getConnectionCallbacks());
    HTDBG("conn cb register done");

    HTDBG("calling bt scan start");
    startScanning();

    HTDBG("ConcreteBLEReceiver::start completed successfully");
  }

  void stop() override
  {
    HTDBG("ConcreteBLEReceiver::stop");
    if (!m_context.getSensorConfiguration().scanningEnabled) {
      HTDBG("Sensor Configuration has scanning disabled. Returning.");
      return;
    }
    
    herald::ble::zephyrinternal::resetReceiverInstance(); // destroys the shared_ptr not necessarily the underlying value

    stopScanning();

    // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
    
    HTDBG("ConcreteBLEReceiver::stop completed successfully");
  }

  // Herald V1 protocol provider overrides
  // C++17 CALLBACK VERSION:-
  // void openConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void closeConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void serviceDiscovery(Activity, CompletionCallback) override;
  // void readPayload(Activity, CompletionCallback) override;
  // void immediateSend(Activity, CompletionCallback) override;
  // void immediateSendAll(Activity, CompletionCallback) override;
  
  // NON C++17 VERSION:-
  bool openConnection(const TargetIdentifier& toTarget) override
  {
    HTDBG("openConnection");

    // Create addr from TargetIdentifier data
    ConnectedDeviceState& state = findOrCreateState(toTarget);
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
    HTDBG("Address copied. Constituted as:-");
    // idiot check of copied data
    Data newAddr(state.address.a.val,6);
    BLEMacAddress newMac(newAddr);
    HTDBG((std::string)newMac);




    // // print out device info
    // BLEMacAddress mac(addrData);
    // std::string di("Opening Connection :: Device info: mac=");
    // di += (std::string)mac;
    // di += ", os=";
    // auto devPtr = db.device(toTarget);
    // auto os = devPtr->operatingSystem();
    // if (os.has_value()) {
    //   if (herald::ble::BLEDeviceOperatingSystem::ios == os) {
    //     di += "ios";
    //   } else if (herald::ble::BLEDeviceOperatingSystem::android == os) {
    //     di += "android";
    //   }
    // } else {
    //   di += "unknown";
    // }
    // di += ", ignore=";
    // auto ignore = devPtr->ignore();
    // if (ignore) {
    //   di += "true";
    // } else {
    //   di += "false";
    // }

    // HTDBG(di);
    
    
    // temporarily stop scan - WORKAROUND for https://github.com/zephyrproject-rtos/zephyr/issues/20660
    // HTDBG("pausing scanning");
    stopScanning();
    m_context.getPlatform().getAdvertiser().stopAdvertising();
    // HTDBG("Scanning paused");


    // attempt connection, if required
    bool ok = true;
    if (NULL == state.connection) {
      HTDBG(" - No existing connection. Attempting to connect");
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
      // HTDBG(os.str());
      // HTDBG(std::to_string(zephyrinternal::defaultCreateParam.interval));
      // HTDBG(std::to_string(zephyrinternal::defaultCreateParam.window));
      // HTDBG(std::to_string(zephyrinternal::defaultCreateParam.timeout));
      // HTDBG(std::to_string(zephyrinternal::defaultConnParam.interval_min));
      // HTDBG(std::to_string(zephyrinternal::defaultConnParam.interval_max));
      // HTDBG(std::to_string(zephyrinternal::defaultConnParam.latency));
      // HTDBG(std::to_string(zephyrinternal::defaultConnParam.timeout));
      // HTDBG("Random address check ok?");
      // HTDBG(bt_le_scan_random_addr_check() ? "yes" : "no");
      
      char addr_str[BT_ADDR_LE_STR_LEN];
      bt_addr_le_to_str(&state.address, addr_str, sizeof(addr_str));
      HTDBG("ADDR AS STRING in openConnection:-");
      HTDBG(addr_str);

      state.state = BLEDeviceState::connecting; // this is used by the condition variable
      state.remoteInstigated = false; // as we're now definitely the instigators
      int success = bt_conn_le_create(
        &state.address,
        zephyrinternal::getDefaultCreateParam(),
        zephyrinternal::getDefaultConnParam(),
        &state.connection
      );
      HTDBG(" - post connection attempt");
      if (0 != success) {
        ok = false;
        if (-EINVAL == success) {
          HTDBG(" - ERROR in passed in parameters");
        } else if (-EAGAIN == success) {
          HTDBG(" - bt device not ready");
        } else if (-EALREADY == success) {
          HTDBG(" - bt device initiating")
        } else if (-ENOMEM == success) {
          HTDBG(" - bt connect attempt failed with default BT ID. Trying again later.");
          // auto device = db.device(toTarget);
          // device->ignore(true);
        } else if (-ENOBUFS == success) {
          HTDBG(" - bt_hci_cmd_create has no buffers free");
        } else if (-ECONNREFUSED == success) {
          HTDBG(" - Connection refused");
        } else if (-EIO == success) {
          HTDBG(" - Low level BT HCI opcode IO failure");
        } else {
          HTDBG(" - Unknown error code...");
          HTDBG(std::to_string(success));
        }

        // Add to ignore list for now
        // DONT DO THIS HERE - MANY REASONS IT CAN FAIL auto device = db.device(toTarget);
        // HTDBG(" - Ignoring following target: {}", toTarget);
        // device->ignore(true);
        
        // Log last disconnected time in BLE database (records failure, allows progressive backoff)
        auto device = db.device(newMac); // Find by actual current physical address
        device->state(BLEDeviceState::disconnected);
        
        // Immediately restart advertising on failure, but not scanning
        m_context.getPlatform().getAdvertiser().startAdvertising();

        return false;
      } else {
        HTDBG("Zephyr waitWithTimeout for new connection");
        // lock and wait for connection to be created
        
        // STD::ASYNC/MUTEX variant:-
        // std::unique_lock<std::mutex> lk(bleInUse);
        // connectionAvailable.wait(lk, [this] {
        //   return connectionState == BLEDeviceState::connecting;
        // }); // BLOCKS
        // verify connection successful
        // connCallback(toTarget,connectionState == BLEDeviceState::connected);

        // ZEPHYR SPECIFIC VARIANT
        uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&state] {
          return state.state == BLEDeviceState::connecting;
        });
        if (timedOut != 0) {
          HTDBG("ZEPHYR WAIT TIMED OUT. Is connected?");
          HTDBG((state.state == BLEDeviceState::connected) ? "true" : "false");
          HTDBG(std::to_string(timedOut));
          return false;
        }
        // return connectionState == BLEDeviceState::connected;
        return state.state == BLEDeviceState::connected;
      }
    } else {
      HTDBG(" - Existing connection exists! Reusing.");
      return true;
    }
  }



  bool closeConnection(const TargetIdentifier& toTarget) override
  {
    HTDBG("closeConnection call for ADDR:-");
    ConnectedDeviceState& state = findOrCreateState(toTarget);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&state.address, addr_str, sizeof(addr_str));
    HTDBG(addr_str);
    if (NULL != state.connection) {
      if (state.remoteInstigated) {
        HTDBG("Connection remote instigated - not forcing close");
      } else {
        bt_conn_disconnect(state.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        // auto device = db.device(toTarget);
        // device->registerDisconnect(Date());
      }
    } else {
      // Can clear the remote instigated flag as they've closed the connection
      state.remoteInstigated = false;
    }
    if (!state.remoteInstigated) {
      removeState(toTarget);
      return false; // assumes we've closed it // TODO proper multi-connection state tracking
    }
    return true; // remote instigated the connection - keep it open and inform caller
  }



  void restartScanningAndAdvertising() override
  {
    // Print out current list of devices and their info
    if (!connectionStates.empty()) {
      HTDBG("Current connection states cached:-");
      for (auto& [key,value] : connectionStates) {
        std::string ci = " - ";
        ci += ((Data)value.target).hexEncodedString();
        ci += " state: ";
        switch (value.state) {
          case BLEDeviceState::connected:
            ci += "connected";
            break;
          case BLEDeviceState::disconnected:
            ci += "disconnected";
            break;
          default:
            ci += "connecting";
        }
        ci += " connection is null: ";
        ci += (NULL == value.connection ? "true" : "false");
        HTDBG(ci);

        // Check connection reference is valid by address - has happened with non connectable devices (VR headset bluetooth stations)
        value.connection = bt_conn_lookup_addr_le(BT_ID_DEFAULT, &value.address);
        // If the above returns null, the next iterator will remove our state

        // Check for non null connection but disconnected state
        
        if (BLEDeviceState::disconnected == value.state) {
          value.connection = NULL;
        }
        // Now check for timeout - nRF Connect doesn't cause a disconnect callback
        if (NULL != value.connection && value.remoteInstigated) {
          HTDBG("REMOTELY INSTIGATED OR CONNECTED DEVICE TIMED OUT");
          auto device = db.device(value.target);
          if (device->timeIntervalSinceConnected() < TimeInterval::never() &&
              device->timeIntervalSinceConnected() > TimeInterval::seconds(30)) {
            // disconnect
            bt_conn_disconnect(value.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            value.connection = NULL;
          }
        }
      }

      // Do internal clean up too - remove states no longer required
      for (auto iter = connectionStates.begin();connectionStates.end() != iter; ++iter) {
        if (NULL == iter->second.connection) { // means Zephyr callbacks are finished with the connection object (i.e. disconnect was called)
          connectionStates.erase(iter);
        }
      }
    }

    // Restart scanning
    // HTDBG("restartScanningAndAdvertising - requesting scanning and advertising restarts");
    startScanning();
    m_context.getPlatform().getAdvertiser().startAdvertising();
  }

  std::optional<Activity> serviceDiscovery(Activity activity) override
  {
    auto currentTargetOpt = std::get<1>(activity.prerequisites.front());
    if (!currentTargetOpt.has_value()) {
      HTDBG("No target specified for serviceDiscovery activity. Returning.");
      return {}; // We've been asked to connect to no specific target - not valid for Bluetooth
    }
    // Ensure we have a cached state (i.e. we are connected)
    auto& state = findOrCreateState(currentTargetOpt.value());
    if (state.state != BLEDeviceState::connected) {
      HTDBG("Not connected to target of activity. Returning.");
      return {};
    }
    if (NULL == state.connection) {
      HTDBG("State for activity does not have a connection. Returning.");
      return {};
    }
    auto device = db.device(currentTargetOpt.value());

    gatt_discover(state.connection);

    uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&device] () -> bool {
      return !device->hasServicesSet(); // service discovery not completed yet
    });

    if (0 != timedOut) {
      HTDBG("service discovery timed out for device");
      HTDBG(std::to_string(timedOut));
      return {};
    }
    return {};
  }

  std::optional<Activity> readPayload(Activity activity) override
  {
    return {};
  }

  std::optional<Activity> immediateSend(Activity activity) override
  {
    return {};
  }
  std::optional<Activity> immediateSendAll(Activity activity) override
  {
    return {};
  }

private:  
  // Zephyr OS callbacks
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
      struct net_buf_simple *buf) override
  {
    // identify device by both MAC and potential pseudoDeviceAddress
    BLEMacAddress bleMacAddress(addr->a.val);
    Data advert(buf->data,buf->len);
    auto device = db.device(bleMacAddress,advert);

    // auto device = db.device(target);
    if (device->ignore()) {
      // device->rssi(RSSI(rssi)); // TODO should we do this so our update date works and shows this as a 'live' device?
      return;
    }

    // // Now pass to relevant BLEDatabase API call
    if (!device->rssi().has_value()) {
      char addr_str[BT_ADDR_LE_STR_LEN];
      bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
      std::string addrStr(addr_str);
      HTDBG("New address FROM SCAN:-");
      HTDBG(addr_str);
    }

    // Add this RSSI reading - called at the end to ensure all other data variables set
    device->rssi(RSSI(rssi));
  }

  void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) override
  {
    HTDBG("le param updated called");
  }

  void connected(struct bt_conn *conn, uint8_t err) override
  {
    HTDBG("**************** Zephyr connection callback. Mac of connected:");

    auto addr = bt_conn_get_dst(conn);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    std::string addrStr(addr_str);
    BLEMacAddress bleMacAddress(addr->a.val);
    HTDBG((std::string)bleMacAddress);

    ConnectedDeviceState& state = findOrCreateStateByConnection(conn, true);
    auto device = db.device(bleMacAddress); // Find by actual current physical address

    if (err) { // 2 = SMP issues? StreetPass blocker on Android device perhaps. Disabled SMP use?
      // When connecting to some devices (E.g. HTC Vive base station), you will connect BUT get an error code
      // The below ensures that this is counted as a connection failure

      HTDBG("Connected: Error value:-");
      HTDBG(std::to_string(err));
      // Note: See Bluetooth Specification, Vol 2. Part D (Error codes)

      bt_conn_unref(conn);
      
      state.state = BLEDeviceState::disconnected;
      state.connection = NULL;
        
      // Log last disconnected time in BLE database
      device->state(BLEDeviceState::disconnected);

      // if (targetForConnection.has_value() && connCallback.has_value()) {
      //   connCallback.value()(targetForConnection.value(),false);
      // }
      return;
    }

    state.connection = conn;
    bt_addr_le_copy(&state.address,addr);
    state.state = BLEDeviceState::connected;

    // Log last connected time in BLE database
    device->state(BLEDeviceState::connected);

    
    // if (targetForConnection.has_value() && connCallback.has_value()) {
    //   connCallback.value()(targetForConnection.value(),true);
    // }

  }

  void disconnected(struct bt_conn *conn, uint8_t reason) override
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
      // Note: See Bluetooth Specification, Vol 2. Part D (Error codes)
      // 0x20 = Unsupported LL parameter value
    }
    
    // TODO log disconnection time in ble database
    
    bt_conn_unref(conn);
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);

    state.state = BLEDeviceState::disconnected;
    state.connection = NULL;

    // Log last disconnected time in BLE database
    auto device = db.device(bleMacAddress); // Find by actual current physical address
    device->state(BLEDeviceState::disconnected);
  }
  
  void discovery_completed_cb(struct bt_gatt_dm *dm, void *context) override
  {
    HTDBG("The GATT discovery procedure succeeded");
    const struct bt_gatt_dm_attr *prev = NULL;
    bool found = false;
    ConnectedDeviceState& state = findOrCreateStateByConnection(bt_gatt_dm_conn_get(dm));
    auto device = db.device(state.target);
    do {
      prev = bt_gatt_dm_char_next(dm,prev);
      if (NULL != prev) {
        // Check for match of uuid to a herald read payload char
        struct bt_gatt_chrc *chrc = bt_gatt_dm_attr_chrc_val(prev);

        int matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldPayloadCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald read characteristic. Reading.");
          device->payloadCharacteristic(m_context.getSensorConfiguration().payloadCharacteristicUUID);
          // initialise payload data for this state
          state.readPayload.clear();

          // if match, for a read
          found = true;
          // set handles

          // TODO REFACTOR THE ACTUAL FETCHING OF PAYLOAD TO READPAYLOAD FUNCTION
          //  - Actually important, as currently a wearable will request the char multiple times from iOS before a reply is received
          zephyrinternal::getReadParams()->single.handle = chrc->value_handle;
          zephyrinternal::getReadParams()->single.offset = 0x0000; // gets changed on each use
          int readErr = bt_gatt_read(bt_gatt_dm_conn_get(dm), zephyrinternal::getReadParams());
          if (readErr) {
            HTDBG("GATT read error: TBD");//, readErr);
            // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
          }

          continue; // check for other characteristics too
        }
        matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldSignalAndroidCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald android signal characteristic. logging.");
          device->signalCharacteristic(m_context.getSensorConfiguration().androidSignalCharacteristicUUID);
          device->operatingSystem(BLEDeviceOperatingSystem::android);

          continue; // check for other characteristics too
        }
        matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldSignalIOSCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald ios signal characteristic. logging.");
          device->signalCharacteristic(m_context.getSensorConfiguration().iosSignalCharacteristicUUID);
          device->operatingSystem(BLEDeviceOperatingSystem::ios);

          continue; // check for other characteristics too
        }
        // otherwise
        char uuid_str[32];
        bt_uuid_to_str(chrc->uuid,uuid_str,sizeof(uuid_str));
        HTDBG("    - Char doesn't match any herald char uuid:-"); //, log_strdup(uuid_str));
        HTDBG(uuid_str);
      }
    } while (NULL != prev);

    if (!found) {
      HTDBG("Herald read payload char not found in herald service (weird...). Ignoring device.");
      device->ignore(true);
      // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }

    // No it doesn't - this is safe: does ending this here break our bt_gatt_read? (as it uses that connection?)
    int err = bt_gatt_dm_data_release(dm);
    if (err) {
      HTDBG("Could not release the discovery data, error code: TBD");
      // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }

    // very last action - for concurrency reasons (C++17 threading/mutex/async/future not available on Zephyr)
    std::vector<UUID> serviceList;
    serviceList.push_back(m_context.getSensorConfiguration().serviceUUID);
    device->services(serviceList);
  }

  void discovery_service_not_found_cb(struct bt_conn *conn, void *context) override
  {
    HTDBG("The service could not be found during the discovery. Ignoring device:");
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);
    HTDBG((std::string)state.target);

    auto device = db.device(state.target);
    std::vector<UUID> serviceList; // empty service list // TODO put other listened-for services here
    device->services(serviceList);
    device->ignore(true);
  }

  void discovery_error_found_cb(struct bt_conn *conn, int err, void *context) override
  {
    HTDBG("The discovery procedure failed with ");
    HTDBG(std::to_string(err));
    // TODO decide if we should ignore the device here, or just keep trying
  }

  uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length) override
  {
    // Fetch state for this element
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);
    if (NULL == data) {
      HTDBG("Finished reading CHAR read payload:-");
      HTDBG(state.readPayload.hexEncodedString());
      
      // Set final read payload (triggers success callback on observer)
      db.device(state.target)->payloadData(state.readPayload);

      return 0;
    }

    state.readPayload.append((const uint8_t*)data,0,length);
    return length;
  }

  // std::optional<ConnectedDeviceState&> findState(const TargetIdentifier& forTarget);
  // std::optional<ConnectedDeviceState&> findStateByConnection(struct bt_conn *conn);
  ConnectedDeviceState& findOrCreateState(const TargetIdentifier& forTarget)
  {
    auto iter = connectionStates.find(forTarget);
    if (connectionStates.end() != iter) {
      return iter->second;
    }
    return connectionStates.emplace(forTarget, forTarget).first->second;
    // return connectionStates.find(forTarget)->second;
  }

  ConnectedDeviceState& findOrCreateStateByConnection(struct bt_conn *conn, bool remoteInstigated = false)
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
    result.first->second.remoteInstigated = remoteInstigated;
    return result.first->second;
  }

  void removeState(const TargetIdentifier& forTarget)
  {
    auto iter = connectionStates.find(forTarget);
    if (connectionStates.end() != iter) {
      connectionStates.erase(iter);
    }
  }

  // internal call methods
  void startScanning()
  {
    if (isScanning) {
      return;
    }
    int err = bt_le_scan_start(zephyrinternal::getDefaultScanParam(), &zephyrinternal::scan_cb); // scan_cb linked via BT_SCAN_CB_INIT call
    
    if (0 != err) {
      HTDBG("Starting scanning failed");
      return;
    }
    isScanning = true;
  }
  
  void stopScanning()
  {
    if (isScanning) {
      isScanning = false;
      bt_le_scan_stop();
    }
  }

  void gatt_discover(struct bt_conn *conn)
  {
    HTDBG("Attempting GATT service discovery");
    int err;

    // begin introspection
    err = bt_gatt_dm_start(conn, &zephyrinternal::getHeraldUUID()->uuid, zephyrinternal::getDiscoveryCallbacks(), NULL);
    if (err) {
      HTDBG("could not start the discovery procedure, error code")
      HTDBG(std::to_string(err));
      bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN); // ensures disconnect() called, and loop completed
      return;
    }
    HTDBG("Service discovery succeeded... now do something with it in the callback!");
  }

  ContextT& m_context;
  BluetoothStateManager& m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  BLEDatabaseT& db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  std::map<TargetIdentifier,ConnectedDeviceState> connectionStates;
  bool isScanning;

  HLOGGER(ContextT);
};

}
}

#endif