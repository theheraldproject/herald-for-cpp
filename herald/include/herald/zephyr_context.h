//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ZEPHYR_CONTEXT_H
#define ZEPHYR_CONTEXT_H

#include "context.h"
#include "ble/bluetooth_state_manager.h"

// Herald logging to zephyr - see zephyr_context.cpp for details
#ifndef CONFIG_HERALD_LOG_LEVEL
  #define CONFIG_HERALD_LOG_LEVEL 0
#endif

#include "data/zephyr/zephyr_logging_sink.h"
#include "data/sensor_logger.h"

#include <memory>
#include <iosfwd>
#include <string>
#include <functional>
#include <optional>

#include <bluetooth/bluetooth.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>

namespace herald {

using namespace herald::ble;
using namespace herald::data;

/// \brief Internal zephyr namespace DO NOT USE - API MAY CHANGE WITHOUT WARNING
namespace zephyrinternal {
  class Advertiser {
  public:
    Advertiser();
    ~Advertiser();
    void stopAdvertising() noexcept;
    void startAdvertising() noexcept;
    void registerStopCallback(std::function<void()> cb);
    void registerStartCallback(std::function<void()> cb);
  private:
    std::optional<std::function<void()>> startCallback;
    std::optional<std::function<void()>> stopCallback;
  };
}

///
/// \brief Holds generic state across our application for any Zephyr RTOS device.
/// 
/// Provides a solid class that holds information and types to be pass to Context.
///
/// Also acts as the Zephyr PlatformT in Context.
///
class ZephyrContextProvider : BluetoothStateManager {
public:
  ZephyrContextProvider();
  ~ZephyrContextProvider();

  ZephyrLoggingSink& getLoggingSink();
  BluetoothStateManager& getBluetoothStateManager();

  // Bluetooth State Manager override methods
  void add(BluetoothStateManagerDelegate& delegate) override;
  BluetoothState state() override;

  // Zephyr OS specific methods
  int enableBluetooth() noexcept;

  herald::zephyrinternal::Advertiser& getAdvertiser() noexcept;

  int startBluetooth() noexcept;
  int stopBluetooth() noexcept;

  void periodicActions() noexcept;

private:
  ZephyrLoggingSink sink;

  herald::zephyrinternal::Advertiser advertiser;

  std::vector<std::reference_wrapper<BluetoothStateManagerDelegate>> stateDelegates;

  bool bluetoothEnabled;
};


// Other zephyr internal-but-public API base classes
namespace zephyrinternal {

/// \brief INTERNAL utility class to allow Zephyr C API to call callbacks in the Zephyr internal Context Impl class.
class Callbacks {
public:
  Callbacks() = default;
  virtual ~Callbacks() = default;

  // Scanning
  virtual void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
      struct net_buf_simple *buf) = 0;
  
  // GATT Discovery
  virtual void discovery_completed_cb(struct bt_gatt_dm *dm, void *context) = 0;
  virtual void discovery_service_not_found_cb(struct bt_conn *conn, void *context) = 0;
  virtual void discovery_error_found_cb(struct bt_conn *conn, int err, void *context) = 0;
  virtual uint8_t gatt_read_cb(struct bt_conn *conn, uint8_t err,
              struct bt_gatt_read_params *params,
              const void *data, uint16_t length) = 0;

  // Connection management
  virtual void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) = 0;
  virtual void connected(struct bt_conn *conn, uint8_t err) = 0;
  virtual void disconnected(struct bt_conn *conn, uint8_t reason) = 0;
};

}

} // end namespace

#endif