//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ZEPHYR_CONTEXT_H
#define ZEPHYR_CONTEXT_H

#include "context.h"
#include "ble/bluetooth_state_manager.h"

#include <memory>
#include <iosfwd>
#include <string>

#include <bluetooth/bluetooth.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>

// Herald logging to zephyr - see zephyr_context.cpp for details
#ifndef CONFIG_HERALD_LOG_LEVEL
  #define CONFIG_HERALD_LOG_LEVEL 0
#endif

namespace herald {

using namespace herald::ble;

/*
 * Zephyr context class - holds state generic across our application for a particular device.
 */
class ZephyrContext : public Context, public BluetoothStateManager, public std::enable_shared_from_this<ZephyrContext> {
public:
  ZephyrContext();
  ~ZephyrContext();

  // Context override methods
  std::shared_ptr<SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor) override;
  std::shared_ptr<BluetoothStateManager> getBluetoothStateManager() override;

  // Bluetooth State Manager override methods
  void add(std::shared_ptr<BluetoothStateManagerDelegate> delegate) override;
  BluetoothState state() override;

  // Zephyr OS specific methods
  int enableBluetooth() noexcept;

  int startBluetooth() noexcept;
  int stopBluetooth() noexcept;

  void periodicActions() noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL idiom
};


// Other zephyr internal-but-public API base classes
namespace zephyrinternal {

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

  // Connection management
  virtual void le_param_updated(struct bt_conn *conn, uint16_t interval,
            uint16_t latency, uint16_t timeout) = 0;
  virtual void connected(struct bt_conn *conn, uint8_t err) = 0;
  virtual void disconnected(struct bt_conn *conn, uint8_t reason) = 0;
};

}

} // end namespace

#endif