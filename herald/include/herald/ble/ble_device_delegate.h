//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_DEVICE_DELEGATE_H
#define HERALD_BLE_DEVICE_DELEGATE_H

#include "ble_device.h"

#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

class BLEDeviceDelegate {
public:
  BLEDeviceDelegate() = default;
  virtual ~BLEDeviceDelegate() = default;

  virtual void device(const BLEDevice& device, const BLEDeviceAttribute didUpdate) = 0;  
};

} // end namespace
} // end namespace

#endif