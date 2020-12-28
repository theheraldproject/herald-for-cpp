//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLUETOOTH_STATE_MANAGER_H
#define BLUETOOTH_STATE_MANAGER_H

#include "../datatype/bluetooth_state.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// Tagging interface
class BluetoothStateManager {
public:
  BluetoothStateManager() = default;
  virtual ~BluetoothStateManager() = default;

  virtual BluetoothState state() = 0;
};

} // end namespace
} // end namespace

#endif