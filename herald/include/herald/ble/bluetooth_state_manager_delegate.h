//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLUETOOTH_STATE_MANAGER_DELEGATE_H
#define HERALD_BLUETOOTH_STATE_MANAGER_DELEGATE_H

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

namespace std {

bool operator!=(const herald::ble::BluetoothStateManagerDelegate& lhs,const herald::ble::BluetoothStateManagerDelegate& rhs) noexcept;

}

#endif