//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLUETOOTH_STATE_MANAGER_DELEGATE_H
#define BLUETOOTH_STATE_MANAGER_DELEGATE_H

#include "../datatype/bluetooth_state.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// Tagging interface
class BluetoothStateManagerDelegate {
public:
  BluetoothStateManagerDelegate() = default;
  virtual ~BluetoothStateManagerDelegate() = default;

  virtual void bluetoothStateManager(BluetoothState didUpdateState) = 0;
};

} // end namespace
} // end namespace

#endif