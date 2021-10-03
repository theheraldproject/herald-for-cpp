//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ZEPHYR_CONTEXT_H
#define HERALD_ZEPHYR_CONTEXT_H

#include "context.h"
#include "ble/bluetooth_state_manager.h"

// Herald logging to zephyr - see zephyr_context.cpp for details
#ifndef CONFIG_HERALD_LOG_LEVEL
  #define CONFIG_HERALD_LOG_LEVEL 0
#endif

#include "data/zephyr/zephyr_logging_sink.h"
#include "data/sensor_logger.h"
#include "datatype/allocatable_array.h"

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

// FWD DECL
class ZephyrContextProvider;

/// \brief Internal zephyr namespace DO NOT USE - API MAY CHANGE WITHOUT WARNING
namespace zephyrinternal {
  class Advertiser {
  friend class herald::ZephyrContextProvider; // gives direct access to customServices private variable
  public:
    Advertiser() noexcept;
    ~Advertiser() noexcept;
    /// \brief Stop advertising
    void stopAdvertising() noexcept;
    /// \brief Start advertising if Bluetooth is enabled
    void startAdvertising() noexcept;
    /// \brief Restart advertising if, and only if, it is already advertising
    void restartAdvertising() noexcept;
    /// \brief Inform the advertiser that its advert needs refreshing. Does not force the advert to change immediately.
    /// See restartAdvertising for that
    void markAdvertAsDirty() noexcept;

    /// \brief Used by a Bluetooth transmitter to register a callback to tell it when to stop advertising
    void registerStopCallback(std::function<void()> cb) noexcept;
    /// \brief Used by a Bluetooth transmitter to register a callback to tell it when to start advertising
    void registerStartCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept;
    /// \brief Used by a Bluetooth transmitter to register a callback to tell it when to restart advertising
    void registerRestartCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept;
    /// \brief Used by a Bluetooth transmitter to register a callback to tell it when its advert is dirty
    void registerIsDirtyCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept;
    /// \brief Safety function to deregister callbacks when transmitter shuts down
    void unregisterAllCallbacks() noexcept;
  private:
    std::optional<std::function<void()>> stopCallback;
    std::optional<std::function<void(BLEServiceList&)>> startCallback;
    std::optional<std::function<void(BLEServiceList&)>> restartCallback;
    std::optional<std::function<void(BLEServiceList&)>> isDirtyCallback;
    BLEServiceList customServices;
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

  Date getNow() noexcept;

  // MARK: v2.1 Bluetooth State Manager functions
  
  bool addCustomService(const herald::ble::BluetoothUUID& serviceId) override;

  void removeCustomService(const herald::ble::BluetoothUUID& serviceId) override;

  bool addCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::ble::BLECharacteristicType& charType, const herald::ble::BLECallbacks& callbacks) override;

  void removeCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId) override;

  void notifyAllSubscribers(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue) override;

  void notifySubscriber(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue, const herald::ble::BLEMacAddress& toNotify) override;

private:
  ZephyrLoggingSink sink;

  herald::zephyrinternal::Advertiser advertiser;

  ReferenceArray<BluetoothStateManagerDelegate> stateDelegates;

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

  // Internal info callbacks
  virtual void print(struct bt_conn *conn, void *data) = 0;
  virtual void close(struct bt_conn *conn, void *data) = 0;

  // Connection management
  virtual void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) = 0;
  virtual void connected(struct bt_conn *conn, uint8_t err) = 0;
  virtual void disconnected(struct bt_conn *conn, uint8_t reason) = 0;
};

}

} // end namespace

#endif