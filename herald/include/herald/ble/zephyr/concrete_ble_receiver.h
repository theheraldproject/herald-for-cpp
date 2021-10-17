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
      readPayload(), immediateSend(), remoteInstigated(false), inDiscovery(false), isReading(false)
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
  bool inDiscovery;
  bool isReading;
};

namespace zephyrinternal {
  std::string toMacString(const bt_addr_le_t* addr);
  std::string toIdentityString(const bt_addr_le_t* addr);
  
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

  [[maybe_unused]]
  void print_cb(struct bt_conn *conn, void *data);

  [[maybe_unused]]
  void close_cb(struct bt_conn *conn, void *data);

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

template <typename ContextT, typename PayloadDataSupplierT, typename BLEDatabaseT, typename SensorDelegateSetT>
class ConcreteBLEReceiver : public HeraldProtocolV1Provider, public herald::zephyrinternal::Callbacks /*, public std::enable_shared_from_this<ConcreteBLEReceiver<ContextT>>*/ {
public:
  ConcreteBLEReceiver(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    PayloadDataSupplierT& payloadDataSupplier, BLEDatabaseT& bleDatabase, SensorDelegateSetT& dels)
    : m_context(ctx), // Herald API guarantees this to be safe
      m_stateManager(bluetoothStateManager),
      m_pds(payloadDataSupplier),
      db(bleDatabase),
      delegates(dels),
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
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider()
  {
    return {}; // we don't provide this, ConcreteBLESensor provides this. We provide HeraldV1ProtocolProvider
  }

  // bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override
  // {
  //   return false;
  // }

  // bool immediateSendAll(Data data) override {
  //   return false;
  // }

  // Sensor overrides
  void start()
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
      HTDBG("ERROR starting context bluetooth: {}", startOk);
    }

    HTDBG("Calling conn cb register");
    bt_conn_cb_register(zephyrinternal::getConnectionCallbacks());
    HTDBG("conn cb register done");

    HTDBG("calling bt scan start");
    startScanning();

    HTDBG("ConcreteBLEReceiver::start completed successfully");
  }

  void stop()
  {
    HTDBG("ConcreteBLEReceiver::stop");
    if (!m_context.getSensorConfiguration().scanningEnabled) {
      HTDBG("Sensor Configuration has scanning disabled. Returning.");
      return;
    }
    
    herald::ble::zephyrinternal::resetReceiverInstance(); // may not destroy the underlying value

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

  void print(struct bt_conn *conn,void *data) override {
    struct bt_conn_info info;
    int success = bt_conn_get_info(conn,&info);
    if (0 == success) {
      HTDBG("  {}:{}, t: {}0ms, local: {}({}), remote: {}({})",
        info.id,
        (BT_CONN_TYPE_LE==info.type?"LE":"BR"),
        info.le.timeout,
        zephyrinternal::toMacString(info.le.local),
        zephyrinternal::toIdentityString(info.le.src),
        zephyrinternal::toMacString(info.le.remote),
        zephyrinternal::toMacString(info.le.dst)
      );
    } else {
      HTDBG("  Error reading connection info");
    }
  }

  void printZephyrConnectionStates() {
    HTDBG("EXISTING CONNECTION INFO:-");
    bt_conn_foreach(BT_CONN_TYPE_LE, zephyrinternal::print_cb, NULL);
  }

  void close(struct bt_conn *conn, void *data) override {
    bt_conn_disconnect(conn,BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    // bt_conn_unref(conn);
  }

  void forceCloseAll() {
    bt_conn_foreach(BT_CONN_TYPE_LE, zephyrinternal::close_cb, NULL);
  }

  // NON C++17 VERSION:-
  bool openConnection(const TargetIdentifier& toTarget) override
  {
    HTDBG("openConnection");

    printZephyrConnectionStates(); // zephyr's low-level view of live connections
    printAllStates(); // our state engine cache in this class (holds closed connections)

    // Create addr from TargetIdentifier data
    ConnectedDeviceState& state = findOrCreateState(toTarget);
    uint8_t val[6] = {0,0,0,0,0,0};
    Data addrData = toTarget.underlyingData(); // TODO change this to mac for target ID
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
    // idiot check of copied data
    Data newAddr(state.address.a.val,6);
    BLEMacAddress newMac(newAddr);
    HTDBG("Address copied. Constituted as: {}", (std::string)newMac);




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
    m_context.getPlatform().getAdvertiser().stopAdvertising(); // Forced via ble coordinator, and zephyr internals, not this method
    // TODO investigate le_ext_adv etc to auto start/stop advertising properly
    // HTDBG("Scanning paused");


    // attempt connection, if required
    bool ok = true;
    if (NULL == state.connection) {
      HTDBG(" - No existing connection cached in ConcreteBLEReceiver. Attempting to connect");
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
      HTDBG("ADDR AS STRING in openConnection: {}", addr_str);

      state.state = BLEDeviceState::connecting; // this is used by the condition variable
      state.remoteInstigated = false; // as we're now definitely the instigators
      int success = bt_conn_le_create(
        &state.address,
        zephyrinternal::getDefaultCreateParam(),
        zephyrinternal::getDefaultConnParam(),
        &state.connection
      );
      HTDBG(" - post connection attempt");
      auto& device = db.device(newMac); // Find by actual current physical address
      if (0 != success) {
        HTDBG("Connection call did not succeed");
        ok = false;
        if (-EINVAL == success) {
          HTDBG(" - ERROR in passed in parameters");
          // NOTE: For whatever reason, having ANY connection reach this point stops advertising working (as if connections are held open)
          // Thus we MUST find a way to FORCE these connections to die, and potentially be recreated later

          // Remove connection by calling disconnected explicitly (in case connection closed elsewhere)
          // IGNORE THIS NOTE: Note: Explicit disconnect removed to allow remote instigated connections to not be killed too soon
          //disconnected(state.connection, success);

          // WHY DOES THE BELOW NOT CLEAR THE STRUCT???

          // Force internal disconnect instead
          bt_conn_disconnect(state.connection,BT_HCI_ERR_REMOTE_USER_TERM_CONN);
          bt_conn_unref(state.connection);

        } else if (-EAGAIN == success) {
          HTDBG(" - bt device not ready");
        } else if (-EALREADY == success) {
          HTDBG(" - bt device initiating");
        } else if (-ENOMEM == success) {
          HTDBG(" - bt connect attempt failed with default BT ID. Trying again later.");
          // auto& device = db.device(toTarget);
          // device.ignore(true);
        } else if (-ENOBUFS == success) {
          HTDBG(" - bt_hci_cmd_create has no buffers free");
        } else if (-ECONNREFUSED == success) {
          HTDBG(" - Connection refused");
          // Note: Don't ignore as remote device may support few connections
        } else if (-EIO == success) {
          HTDBG(" - Low level BT HCI opcode IO failure");
        } else {
          HTDBG(" - Unknown error code: {}", success);
        }

        // Add to ignore list for now
        // DONT DO THIS HERE - MANY REASONS IT CAN FAIL auto& device = db.device(toTarget);
        // HTDBG(" - Ignoring following target: {}", toTarget);
        // device.ignore(true);
        
        // Log last disconnected time in BLE database (records failure, allows progressive backoff)
        device.state(BLEDeviceState::disconnected); // Ensures device.ignore(true) called for those that fail immediately to connect
        state.connection = NULL;
        state.state = BLEDeviceState::disconnected;
        
        // Immediately restart advertising on failure, but not scanning
        // DO NOT DO THIS - may be MULTIPLE connections, one of which has succeeded, and the below interferes with gatt discovery
        // m_context.getPlatform().getAdvertiser().startAdvertising();

        return false;
      } else {
        HTDBG("Connection call succeeded. Zephyr waitWithTimeout for new connection");
        // lock and wait for connection to be created
        
        // STD::ASYNC/MUTEX variant:-
        // std::unique_lock<std::mutex> lk(bleInUse);
        // connectionAvailable.wait(lk, [this] {
        //   return connectionState == BLEDeviceState::connecting;
        // }); // BLOCKS
        // verify connection successful
        // connCallback(toTarget,connectionState == BLEDeviceState::connected);

        // ZEPHYR SPECIFIC VARIANT
        // REMOVED in v2.1 as Zephyr's own connected callback handles this better
        /*
        uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&state] {
          return state.state == BLEDeviceState::connecting;
        });
        if (timedOut != 0) {
          HTDBG("ZEPHYR WAIT TIMED OUT. Is connected?: {} after {}ms", 
            (state.state == BLEDeviceState::connected) ? "true" : "false",
            timedOut
          );
          state.state = BLEDeviceState::disconnected;
          state.connection = NULL;
          device.state(BLEDeviceState::disconnected); // Ensures device.ignore(true) called for those that connect with no response
          return false;
        }
        // Register success here rather than connected callback, as that is also call when the remote connects to us (wrong direction)
        device.state(BLEDeviceState::connected);
        // return connectionState == BLEDeviceState::connected;
        return state.state == BLEDeviceState::connected;
        */
        // Assume it will work (and accept later errors in service discovery and payload reading)
        return true;
      }
    } else {
      HTDBG(" - Existing connection exists! Reusing.");
      return true;
    }
  }



  bool closeConnection(const TargetIdentifier& toTarget) override
  {
    ConnectedDeviceState& state = findOrCreateState(toTarget);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&state.address, addr_str, sizeof(addr_str));
    HTDBG("closeConnection call for ADDR: {}", addr_str);
    // if (0 == strcmp(addr_str,"00:00:00:00:00:00 (public)")) {
    //   HTDBG("Remote address is empty. Not removing old state object.");
    //   return false; // Assume this Zephyr internal state is being managed out eventually.
    // }
    if (NULL != state.connection) {
      if (state.remoteInstigated) {
        HTDBG("Connection remote instigated - not forcing close");
      } else if (state.inDiscovery) {
        HTDBG("Connection in-use for service discovery - not forcing close");
      } else if (state.isReading) {
        HTDBG("Connection in-use for characteristic read - not forcing close");
      } else {
        bt_conn_disconnect(state.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        bt_conn_unref(state.connection);
        state.connection = NULL;
        // auto& device = db.device(toTarget);
        // device.registerDisconnect(Date());
      }
    } else {
      HTDBG("State connections is null - assuming it is closed");
      // Can clear the remote instigated flag as they've closed the connection
      state.remoteInstigated = false;
      // state.isReading = false;
      // state.readPayload.clear();
    }
    if (!state.remoteInstigated && !state.inDiscovery && !state.isReading) {
      HTDBG("Removing old state connection cache object");
      removeState(toTarget);
      return false; // assumes we've closed it // Multi-connection tracking done elsewhere
    }
    HTDBG("Not removed state cache for connection. Notifying caller connection is not yet closed.");
    return true; // remote instigated the connection - keep it open and inform caller
  }



  void restartScanningAndAdvertising() override
  {
    HTDBG("RESTART SCANNING AND ADVERTISING CALLED");
    // Print out current list of devices and their info
    bool hasInUseConnection = false;
    if (!connectionStates.empty()) {
      HTDBG("Current connection states cached:-");
      for (auto& [key,value] : connectionStates) {
        doStatePrint(key,value);

        // Check connection reference is valid by address - has happened with non connectable devices (VR headset bluetooth stations)
        bool nullBefore = (NULL == value.connection);
        char addr_str[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(&value.address, addr_str, sizeof(addr_str));
        HTDBG("Looking up connection object for address: {}", addr_str);
        value.connection = bt_conn_lookup_addr_le(BT_ID_DEFAULT, &value.address);
        if (!nullBefore && (NULL == value.connection)) {
          HTDBG("  WARNING connection was not null, but is now we've tried to look it up again - WHY? Zephyr could not find connection by address?");
        }
        // If the above returns null, the next iterator will remove our state

        // Check for non null connection but disconnected state
        if (BLEDeviceState::disconnected == value.state) {
          HTDBG("Connection is in disconnected state. Setting state cache connection to NULL");
          value.connection = NULL;
        }
        // Now check for timeout - nRF Connect doesn't cause a disconnect callback
        if (NULL != value.connection && value.remoteInstigated) {
          HTDBG("REMOTELY INSTIGATED OR CONNECTED DEVICE TIMED OUT");
          auto& device = db.device(value.target);
          // if (device.timeIntervalSinceConnected() < TimeInterval::never() &&
          //     device.timeIntervalSinceConnected() > TimeInterval::seconds(30)) {
          // TODO verify this is true when the BLEDevice is in the state we require
          // Force disconnect for all added in v2.1 to ensure that this call has the desired effect (overriding low level connection handling)
          // if (device.timeIntervalSinceLastUpdate() < TimeInterval::never() &&
          //     device.timeIntervalSinceLastUpdate() > TimeInterval::seconds(30)) {
            // disconnect
            bt_conn_disconnect(value.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            bt_conn_unref(value.connection);
            value.connection = NULL;
          // }
        }
      }

      // Do internal clean up too - remove states no longer required
      // NOTE we use iter here because we call erase(iter)
      for (auto iter = connectionStates.begin();connectionStates.end() != iter; ++iter) {
        // We don't check for isReading or is in serviceDiscovery here as this is the catch-all, final, timeout check
        if (NULL != iter->second.connection) {
          // Ones that are not null, but have timed out according to BLE settings (This class doesn't get notified by BLEDatabase)
          auto& device = db.device(iter->second.target);
          // TODO verify this is true when the BLEDevice is in the state we require
          // if (device.timeIntervalSinceConnected() > TimeInterval::seconds(30)) { // Replaced pre v2.1 (No longer track initial connection time separately)
          // Force disconnect for all added in v2.1 to ensure that this call has the desired effect (overriding low level connection handling)
          // if (device.timeIntervalSinceLastUpdate() > TimeInterval::seconds(30)) {
            bt_conn_disconnect(iter->second.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            bt_conn_unref(iter->second.connection);
            iter->second.connection = NULL;
          // } else {
          //   hasInUseConnection = hasInUseConnection && 
          //     (iter->second.inDiscovery || iter->second.remoteInstigated || iter->second.isReading);
          // }
        }

        if (NULL == iter->second.connection) { // means Zephyr callbacks are finished with the connection object (i.e. disconnect was called)
          connectionStates.erase(iter);
        }
      }
    }

    // Force any remaining hidden in zephyr to be closed
    forceCloseAll();

    // Restart scanning
    // HTDBG("restartScanningAndAdvertising - requesting scanning and advertising restarts");
    if (hasInUseConnection) {
      HTDBG("CONNECTIONS IN USE - NOT RESTART ADVERTISING AND SCANNING");
    } else {
      HTDBG("RESTARTING ADVERTISING AND SCANNING");
      startScanning();
      m_context.getPlatform().getAdvertiser().startAdvertising(); // REQUIRED as this is intended to FORCE advertising to start (for remote rssi reads)
    }
  }

  std::optional<Activity> serviceDiscovery(Activity activity) override
  {
    auto currentTargetOpt = std::get<1>(activity.prerequisites.front());
    if (!currentTargetOpt.has_value()) {
      HTERR("No target specified for serviceDiscovery activity. Returning.");
      return {}; // We've been asked to connect to no specific target - not valid for Bluetooth
    }
    // Ensure we have a cached state (i.e. we are connected)
    auto& state = findOrCreateState(currentTargetOpt.value());
    if (state.inDiscovery) {
      HTDBG("Already in discovery. Returning until success or failure.");
      return {};
    }
    if (state.state != BLEDeviceState::connected) {
      HTERR("Not connected to target of activity. Returning.");
      return {};
    }
    if (NULL == state.connection) {
      HTERR("State for activity does not have a connection. Returning.");
      return {};
    }
    [[maybe_unused]]
    auto& device = db.device(currentTargetOpt.value());
    state.inDiscovery = true;

    gatt_discover(state.connection);

    // HTDBG("Zephyr waitWithTimeout for serviceDiscovery");
    // uint32_t timedOut = waitWithTimeout(2'000, K_MSEC(25), [&state] () -> bool {
    //   // return !device.hasServicesSet();
    //   return state.inDiscovery || state.isReading;
    // });
    // HTDBG("Zephyr waitWithTimeout completed for serviceDiscovery");

    // state.inDiscovery = false;

    // if (0 != timedOut) {
    //   BLEMacAddress mac(device.identifier().underlyingData());
    //   HTERR("service discovery timed out for {} after {}ms", ((std::string)mac), timedOut);
    //   return {};
    // }
    return {};
  }

  // Note: The following is initiated within serviceDiscovery's callback as it's gatt related
  std::optional<Activity> readPayload(Activity activity) override
  {
    // HTDBG("Entered readPayload activity");
    // auto currentTargetOpt = std::get<1>(activity.prerequisites.front());
    // if (!currentTargetOpt.has_value()) {
    //   HTDBG("No target specified for Read Payload activity. Returning.");
    //   return {}; // We've been asked to connect to no specific target - not valid for Bluetooth
    // }
    // // Ensure we have a cached state (i.e. we are connected)
    // auto& state = findOrCreateState(currentTargetOpt.value());
    // if (state.state != BLEDeviceState::connected) {
    //   HTDBG("Not connected to target of activity. Returning.");
    //   return {};
    // }
    // if (NULL == state.connection) {
    //   HTDBG("State for activity does not have a connection. Returning.");
    //   return {};
    // }
    // if (!state.isReading) {
    //   HTDBG("Reading already completed. Returning.");
    //   return {};
    // }

    // // Note: Actual read operation started during service discovery, so just wait for it here

    // HTDBG("Zephyr waitWithTimeout for readPayload");
    // uint32_t timedOut = waitWithTimeout(5'000, K_MSEC(25), [&state] () -> bool {
    //   return state.isReading; // service discovery or char reading not completed yet
    // });
    // HTDBG("Zephyr waitWithTimeout completed for readPayload");

    // // Note: Let read continue beyond the time out, so it sets isReading in the read callback once complete (not set to false here)

    // if (0 != timedOut) {
    //   HTDBG("Read Payload timed out for device");
    //   HTDBG(std::to_string(timedOut));
    //   return {};
    // }
    return {};
  }

  // std::optional<Activity> immediateSend(Activity activity) override
  // {
  //   return {};
  // }
  // std::optional<Activity> immediateSendAll(Activity activity) override
  // {
  //   return {};
  // }

private:  
  // Zephyr OS callbacks
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
      struct net_buf_simple *buf) override
  {
    // identify device by both MAC and potential pseudoDeviceAddress
    BLEMacAddress bleMacAddress(addr->a.val);
    Data advert(buf->data,buf->len);
    auto& device = db.device(bleMacAddress,advert);

    // auto& device = db.device(target);
    if (device.ignore()) {
      // device.rssi(RSSI(rssi)); // TODO should we do this so our update date works and shows this as a 'live' device?
      return;
    }

    // Now pass to relevant BLEDatabase API call
    if (device.rssi().intValue() == 0) { // No RSSI yet, so must be a new device instance
      char addr_str[BT_ADDR_LE_STR_LEN];
      bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
      HTDBG("didDiscover (device={})",addr_str);
    }

    // Add this RSSI reading - called at the end to ensure all other data variables set
    device.rssi(RSSI(rssi));
  }

  void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) override
  {
    HTDBG("le param updated called");
  }

  void connected(struct bt_conn *conn, uint8_t err) override
  {

    auto addr = bt_conn_get_dst(conn);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    HTDBG("**************** Zephyr connection callback. Mac of connected: {}", addr_str);
    BLEMacAddress bleMacAddress(addr->a.val);

    ConnectedDeviceState& state = findOrCreateStateByConnection(conn, true);
    auto& device = db.device(bleMacAddress); // Find by actual current physical address

    if (0 != err) { // 2 = SMP issues? StreetPass blocker on Android device perhaps. Disabled SMP use?
      // When connecting to some devices (E.g. HTC Vive base station), you will connect BUT get an error code
      // The below ensures that this is counted as a connection failure

      HTDBG("Connected: Error value: {}", err);
      // Note: See Bluetooth Specification, Vol 2. Part D (Error codes)

      bt_conn_disconnect(state.connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
      bt_conn_unref(conn);
      state.state = BLEDeviceState::disconnected;
      state.connection = NULL;
        
      // Log last disconnected time in BLE database
      device.state(BLEDeviceState::disconnected);

      // if (targetForConnection.has_value() && connCallback.has_value()) {
      //   connCallback.value()(targetForConnection.value(),false);
      // }
      return;
    }
    HTDBG("Connected: Connected successfully");

    // do this here now we don't explicitly wait in the openConnection call - Since v2.1
    device.state(BLEDeviceState::connected);
    state.connection = conn;
    bt_addr_le_copy(&state.address,addr);
    state.state = BLEDeviceState::connected;

    // Log last connected time in BLE database
    // Note: Do this in the timeout callback instead
    //device.state(BLEDeviceState::connected);

    
    // if (targetForConnection.has_value() && connCallback.has_value()) {
    //   connCallback.value()(targetForConnection.value(),true);
    // }

  }

  void disconnected(struct bt_conn *conn, uint8_t reason) override
  {

    auto addr = bt_conn_get_dst(conn);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    HTDBG("********** Zephyr disconnection callback. Mac of disconnected: {}", addr_str);
    BLEMacAddress bleMacAddress(addr->a.val);

    // Do this before calling unref
    auto& device = db.device(bleMacAddress); // Find by actual current physical address

    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);

    if (reason) {
      HTDBG("Disconnection: Reason value:-");
      if (19 == reason) {
        HTDBG("  0x13 (19) remote disconnected from us");
        state.remoteInstigated = false;
        // NOTE: The below solves the 'hanging connection issue' so DO NOT CHANGE OR MOVE
        // i.e. <wrn> bt_conn: Found valid connection in disconnected state
        bt_conn_unref(conn); // Unref to signify we don't need this reference any longer either
      } else if (20 == reason) {
        HTDBG("  0x14 (20) remote_device_terminated_connection_due_to_low_resources");
        state.remoteInstigated = false;
        bt_conn_unref(conn); // Unref to signify we don't need this reference any longer either
      } else if (2 == reason) {
        HTDBG("  0x02 (02) Connection does not exist, or connection open request was cancelled.");
      } else if (22 == reason) {
        HTDBG("  0x16 (22) We closed the connection ourselves");
      } else if (62 == reason) {
        HTDBG("  0x3e (62) connection_failed_to_be_established (opened, but no packets from remote");
        // NOTE: The below solves the 'hanging connection issue' so DO NOT CHANGE OR MOVE
        // i.e. <wrn> bt_conn: Found valid connection in disconnected state
        bt_conn_unref(conn); // Unref to signify we don't need this reference any longer either
        // Assume remote doesn't accept connection
        device.ignore(true);
      } else if (8 == reason) {
        HTDBG("  0x08 (08) Connection timeout - Peripheral or Central did not coordinate connection timeout value.");
      } else {
        HTDBG("  Unknown reason code: {}", reason);
      }
      // Note: See Bluetooth Specification, Vol 2. Part D (Error codes)
      // 0x19 = Unknown LMP PDU (Issued if nRF Connect iOS app disconnects from this device)
      // 0x20 = Unsupported LL parameter value
    }
    
    // TODO log disconnection time in ble database
    
    // bt_conn_unref(conn); // Causes issues in openConnection() if we call this here
    state.state = BLEDeviceState::disconnected;
    state.connection = NULL;

    // clear up all (local) state variables to reset for the next interaction (leave local service list copy)
    state.isReading = false;
    state.inDiscovery = false;

    // Log last disconnected time in BLE database
    // TODO find the cause of the following line causing an MPU Fault (Using CONFIG_ARM_MPU=n for now)
    // TODO Also find out why the above code does not result in the connection being able to reconnect in future (opcode 9)
    device.state(BLEDeviceState::disconnected);
  }
  
  void discovery_completed_cb(struct bt_gatt_dm *dm, void *context) override
  {
    const struct bt_gatt_dm_attr *prev = NULL;
    bool found = false;
    ConnectedDeviceState& state = findOrCreateStateByConnection(bt_gatt_dm_conn_get(dm));
    auto& device = db.device(state.target);
    HTDBG("The GATT discovery procedure succeeded for {}", ((std::string)device.identifier()));
    do {
      prev = bt_gatt_dm_char_next(dm,prev);
      if (NULL != prev) {
        // Check for match of uuid to a herald read payload char
        struct bt_gatt_chrc *chrc = bt_gatt_dm_attr_chrc_val(prev);

        int matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldPayloadCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald read characteristic. Reading.");
          device.payloadCharacteristic(m_context.getSensorConfiguration().payloadCharacteristicUUID);
          // initialise payload data for this state
          state.readPayload.clear();
          state.isReading = true;
          state.inDiscovery = false; // Only do this after setting the above (timing surety)

          // if match, for a read
          found = true;
          // set handles

          // TODO REFACTOR THE ACTUAL FETCHING OF PAYLOAD TO READPAYLOAD FUNCTION
          //  - Actually important, as currently a wearable will request the char multiple times from iOS before a reply is received
          zephyrinternal::getReadParams()->single.handle = chrc->value_handle;
          zephyrinternal::getReadParams()->single.offset = 0x0000; // gets changed on each use
          int readErr = bt_gatt_read(bt_gatt_dm_conn_get(dm), zephyrinternal::getReadParams());
          if (readErr) {
            HTDBG("GATT read error code: {}", readErr);
            // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
          }

          continue; // check for other characteristics too
        }
        matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldSignalAndroidCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald android signal characteristic. logging.");
          device.signalCharacteristic(m_context.getSensorConfiguration().androidSignalCharacteristicUUID);
          device.operatingSystem(BLEDeviceOperatingSystem::android);

          continue; // check for other characteristics too
        }
        matches = bt_uuid_cmp(chrc->uuid, &zephyrinternal::getHeraldSignalIOSCharUUID()->uuid);
        if (0 == matches) {
          HTDBG("    - FOUND Herald ios signal characteristic. logging.");
          device.signalCharacteristic(m_context.getSensorConfiguration().iosSignalCharacteristicUUID);
          device.operatingSystem(BLEDeviceOperatingSystem::ios);

          continue; // check for other characteristics too
        }
        // otherwise
        char uuid_str[32];
        bt_uuid_to_str(chrc->uuid,uuid_str,sizeof(uuid_str));
        HTDBG("    - Char doesn't match any herald char uuid: {}", uuid_str);
      }
    } while (NULL != prev);
    state.inDiscovery = false;

    if (!found) {
      HTDBG("Herald read payload char not found in herald service (weird...). Ignoring device.");
      device.ignore(true);
      // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }

    // No it doesn't - this is safe: does ending this here break our bt_gatt_read? (as it uses that connection?)
    int err = bt_gatt_dm_data_release(dm);
    if (err) {
      HTDBG("Could not release the discovery data, error code: {}", err);
      // bt_conn_disconnect(bt_gatt_dm_conn_get(dm), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }

    // very last action - for concurrency reasons (C++17 threading/mutex/async/future not available on Zephyr)
    std::vector<UUID> serviceList;
    serviceList.push_back(m_context.getSensorConfiguration().serviceUUID);
    device.services(serviceList);
  }

  void discovery_service_not_found_cb(struct bt_conn *conn, void *context) override
  {
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);
    // HTDBG((std::string)state.target);
    HTDBG("The service could not be found during the discovery. Ignoring device: {}", 
      (std::string)BLEMacAddress(state.target.underlyingData()));

    auto& device = db.device(state.target);
    std::vector<UUID> serviceList; // empty service list // TODO put other listened-for services here
    device.services(serviceList);
    device.ignore(true);

    // Set this last so the above take effect
    state.inDiscovery = false;
  }

  void discovery_error_found_cb(struct bt_conn *conn, int err, void *context) override
  {
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);
    state.inDiscovery = false;

    auto addr = bt_conn_get_dst(conn);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    HTERR("The discovery procedure for {} failed with: {}", addr_str, err);
    // TODO decide if we should ignore the device here, or just keep trying
  }

  uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length) override
  {
    HTDBG("In gatt_read_cb");
    // Fetch state for this element
    ConnectedDeviceState& state = findOrCreateStateByConnection(conn);
    if (NULL == data) {
      HTDBG("Finished reading CHAR read payload: ", state.readPayload.hexEncodedString());
      
      // Set final read payload (triggers success callback on observer)
      db.device(state.target).payloadData(state.readPayload);

      // reset payload incase of later re-use
      state.readPayload.clear();
      // and set state to completed so we can clear the connection
      state.isReading = false;

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

  void doStatePrint(const TargetIdentifier& key, const ConnectedDeviceState& value) {
    HTDBG("  {} is {}, conn==null?: {}, ri: {}, inDiscovery: {}, isReading: {}",
      (std::string)BLEMacAddress(key.underlyingData()),
      ((BLEDeviceState::connected==value.state) ? "Connected":
        ((BLEDeviceState::disconnected==value.state) ? "Disconnected" :
          ((BLEDeviceState::connecting==value.state) ? "Connecting" : "Uninitialised")
        )
      ),
      (NULL == value.connection) ? "true" : "false",
      value.remoteInstigated ? "true" : "false",
      value.inDiscovery ? "true" : "false",
      value.isReading ? "true" : "false"
    );
  }

  void printAllStates()
  {
    HTDBG("Printing all current Zephyr Concrete BLE Receiver cached states:-");
    for (auto& [key, value] : connectionStates) {
      doStatePrint(key, value);
    }
    HTDBG("Done");
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
    TargetIdentifier target(bleMacAddress.underlyingData());
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
    // TODO support other optional search service IDs from plugins / apps using Herald too
    err = bt_gatt_dm_start(conn, &zephyrinternal::getHeraldUUID()->uuid, zephyrinternal::getDiscoveryCallbacks(), NULL);
    if (err) {
      HTDBG("could not start the discovery procedure, error code: {}", err);
      auto& state = findOrCreateStateByConnection(conn,false);
      // Note: Explicit disconnect removed to allow remote instigated connections to not be killed too soon
      //bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN); // ensures disconnect() called, and loop completed
      //bt_conn_unref(conn);
      state.connection = NULL;
      return;
    }
    HTDBG("Service discovery succeeded... awaiting discovery callback!");
  }

  ContextT& m_context;
  BluetoothStateManager& m_stateManager;
  PayloadDataSupplierT& m_pds;
  BLEDatabaseT& db;

  SensorDelegateSetT& delegates;

  std::map<TargetIdentifier,ConnectedDeviceState> connectionStates;
  bool isScanning;

  HLOGGER(ContextT);
};

}
}

#endif