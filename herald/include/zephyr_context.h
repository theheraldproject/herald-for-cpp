//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ZEPHYR_CONTEXT_H
#define ZEPHYR_CONTEXT_H

// #ifdef __ZEPHYR__

#include "context.h"
#include "ble/bluetooth_state_manager.h"

#include <memory>
#include <iosfwd>
#include <string>

namespace herald {

using namespace herald::ble;

/*
 * Zephyr context class - holds state generic across our application for a particular device.
 */
class ZephyrContext : public Context, public BluetoothStateManager {
public:
  ZephyrContext();
  ~ZephyrContext();

  // Context override methods
  std::ostream& getLoggingSink(const std::string& requestedFor) override;
  std::shared_ptr<BluetoothStateManager> getBluetoothStateManager() override;

  // Bluetooth State Manager override methods
  BluetoothState state() override;

  // Zephyr OS specific methods
  int enableBluetooth();

  int startBluetooth();
  int stopBluetooth();

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL idiom
};




} // end namespace

// #endif

#endif