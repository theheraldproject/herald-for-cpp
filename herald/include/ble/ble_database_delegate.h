//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DATABASE_DELEGATE_H
#define BLE_DATABASE_DELEGATE_H

#include "ble_device.h"

#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class BLEDatabaseDelegate {
public:
  BLEDatabaseDelegate() = default;
  virtual ~BLEDatabaseDelegate() = default;

  virtual void bleDatabaseDidCreate(const std::shared_ptr<BLEDevice>& device) = 0;
  
  virtual void bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice>& device, const std::shared_ptr<BLEDeviceAttribute>& attribute) = 0;
  
  virtual void bleDatabaseDidDelete(const std::shared_ptr<BLEDevice>& device) = 0;
};

} // end namespace
} // end namespace

#endif