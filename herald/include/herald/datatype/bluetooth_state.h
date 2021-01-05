//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLUETOOTH_STATE_H
#define BLUETOOTH_STATE_H

namespace herald {
namespace datatype {

enum class BluetoothState : short {
  unsupported, poweredOn, poweredOff, resetting
};

} // end namespace
} // end namespace

#endif