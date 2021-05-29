//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLUETOOTH_STATE_H
#define HERALD_BLUETOOTH_STATE_H

namespace herald {
namespace datatype {

enum class BluetoothState : short {
  unsupported, poweredOn, poweredOff, resetting
};

} // end namespace
} // end namespace

#endif