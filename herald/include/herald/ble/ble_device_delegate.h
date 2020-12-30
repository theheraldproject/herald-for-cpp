//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DEVICE_DELEGATE_H
#define BLE_DEVICE_DELEGATE_H

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

  //virtual void device(std::shared_ptr<BLEDevice> device, std::shared_ptr<BLEDeviceAttribute> didUpdate) = 0;  
};

} // end namespace
} // end namespace

#endif