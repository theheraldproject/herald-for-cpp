//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/zephyr/concrete_ble_receiver.h"

namespace herald {
namespace ble {

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

namespace zephyrinternal {
  // Items that can only be defined in a single translation unit (cpp file):-

  
  /* Herald Service Variables */
  struct bt_uuid_128 herald_uuid = BT_UUID_INIT_128(
    0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
  );
  struct bt_uuid_128 herald_char_signal_android_uuid = BT_UUID_INIT_128(
    0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
  );
  struct bt_uuid_128 herald_char_signal_ios_uuid = BT_UUID_INIT_128(
    0x63, 0x43, 0x2d, 0xb0, 0xad, 0xa4, 0xf3, 0x8a, 0x9a, 0x4a, 0xe4, 0xea, 0xf2, 0xd5, 0xb0, 0x0e
  );
  struct bt_uuid_128 herald_char_payload_uuid = BT_UUID_INIT_128(
    0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
  );
  struct bt_uuid_128* getHeraldUUID() {
    return &herald_uuid;
  }
  struct bt_uuid_128* getHeraldSignalAndroidCharUUID() {
    return &herald_char_signal_android_uuid;
  }
  struct bt_uuid_128* getHeraldSignalIOSCharUUID() {
    return &herald_char_signal_ios_uuid;
  }
  struct bt_uuid_128* getHeraldPayloadCharUUID() {
    return &herald_char_payload_uuid;
  }
  

  bt_le_conn_param* BTLEConnParam = BT_LE_CONN_PARAM_DEFAULT; // BT_LE_CONN_PARAM(0x018,3200,0,400); // NOT BT_LE_CONN_PARAM_DEFAULT;
  bt_conn_le_create_param* BTLECreateParam = BT_CONN_LE_CREATE_CONN; // BT_CONN_LE_CREATE_PARAM(BT_CONN_LE_OPT_NONE, 0x0010,0x0010);// NOT BT_CONN_LE_CREATE_CONN;

  struct bt_conn_le_create_param defaultCreateParam = BT_CONN_LE_CREATE_PARAM_INIT(
    BT_CONN_LE_OPT_NONE, BT_GAP_SCAN_FAST_INTERVAL, BT_GAP_SCAN_FAST_INTERVAL
  );
  struct bt_le_conn_param defaultConnParam = BT_LE_CONN_PARAM_INIT(
    //BT_GAP_INIT_CONN_INT_MIN, BT_GAP_INIT_CONN_INT_MAX, 0, 400
    //12, 12 // aka 15ms, default from apple documentation
    0x50, 0x50, // aka 80ms, from nRF SDK LLPM sample
    0, 400
  );

  struct bt_conn_le_create_param* getDefaultCreateParam() {
    return &defaultCreateParam;
  }

  struct bt_le_conn_param* getDefaultConnParam() {
    return &defaultConnParam;
  }

  // Note for apple see: https://developer.apple.com/library/archive/qa/qa1931/_index.html
  // And https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf (BLE section)

  [[maybe_unused]]
  struct bt_le_scan_param defaultScanParam = //BT_LE_SCAN_PASSIVE;
  {
		.type       = BT_LE_SCAN_TYPE_PASSIVE, // passive scan
		.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE, // Scans for EVERYTHING
		.interval   = BT_GAP_SCAN_FAST_INTERVAL, // 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
		.window     = BT_GAP_SCAN_FAST_WINDOW // 0x0010, // V.FAST, NOT BT_GAP_SCAN_FAST_INTERVAL - gap.h
	};

  struct bt_le_scan_param* getDefaultScanParam() {
    return &defaultScanParam;
  }
  
  [[maybe_unused]]
	struct bt_scan_init_param scan_init = {
		.scan_param = &defaultScanParam,
		.connect_if_match = false,
		.conn_param = &defaultConnParam
	};

  struct bt_scan_init_param* getScanInitParam() {
    return &scan_init;
  }

  /**
   * Why is this necessary? Traditional pointer-to-function cannot easily
   * and reliably be wrapped with std::function/bind/mem_fn. We also need
   * the Herald API to use subclasses for each platform, necessitating
   * some sort of static bridge. Not pretty, but works and allows us to
   * prevent nullptr problems
   */
  std::optional<std::reference_wrapper<herald::zephyrinternal::Callbacks>> 
    concreteReceiverInstance;

  void setReceiverInstance(herald::zephyrinternal::Callbacks& cbref) {
    concreteReceiverInstance.emplace(cbref);
  }

  void resetReceiverInstance() {
    concreteReceiverInstance.reset();
  }
  
  [[maybe_unused]]
  struct bt_gatt_read_params read_params = {
    .func = gatt_read_cb,
    .handle_count = 1,
    .single = {
      .handle = 0x0000,
      .offset = 0x0000
    }
  };

  struct bt_gatt_read_params* getReadParams() {
    return &read_params;
  }





  // Functions that provide access (defined in the H file, implemented here):-
  
  [[maybe_unused]]
  void connected(struct bt_conn *conn, uint8_t err)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().connected(conn,err);
    }
  }

  [[maybe_unused]]
  void disconnected(struct bt_conn *conn, uint8_t reason)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().disconnected(conn,reason);
    }
  }

  [[maybe_unused]]
  void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().le_param_updated(conn,interval,latency,timeout);
    }
  }
  
  [[maybe_unused]]
	struct bt_conn_cb conn_callbacks = {
		.connected = connected,
		.disconnected = disconnected,
		.le_param_updated = le_param_updated,
	};

  /// \brief Returns singleton connection callback struct reference
  struct bt_conn_cb* getConnectionCallbacks() {
    return &conn_callbacks;
  }



  
  void discovery_completed_cb(struct bt_gatt_dm *dm,
            void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().discovery_completed_cb(dm,context);
    }
  }

  void discovery_service_not_found_cb(struct bt_conn *conn,
              void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().discovery_service_not_found_cb(conn,context);
    }
  }

  void discovery_error_found_cb(struct bt_conn *conn,
              int err,
              void *context)
  {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().discovery_error_found_cb(conn,err,context);
    }
  }

  const struct bt_gatt_dm_cb discovery_cb = {
    .completed = discovery_completed_cb,
    .service_not_found = discovery_service_not_found_cb,
    .error_found = discovery_error_found_cb,
  };

  /// \brief Returns singleton discovery callback struct reference
  const struct bt_gatt_dm_cb* getDiscoveryCallbacks() {
    return &discovery_cb;
  }


  // High level callbacks / access functions
  
  
  uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length)
  {
    if (concreteReceiverInstance.has_value()) {
      return concreteReceiverInstance.value().get().gatt_read_cb(conn,err,params,data,length);
    }
    return length; // say we've consumed the data anyway
  }


  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf) {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value().get().scan_cb(addr,rssi,adv_type,buf);
    }
  }


}


}
}
