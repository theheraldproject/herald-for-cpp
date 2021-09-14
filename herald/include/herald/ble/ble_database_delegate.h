//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_DATABASE_DELEGATE_H
#define HERALD_BLE_DATABASE_DELEGATE_H

#include "ble_device.h"
#include "../datatype/allocatable_array.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// TODO replace this class entirely with a template class and a variant list type

/// \brief BLEDatabaseDelegat tagging class
class BLEDatabaseDelegate {
public:
  BLEDatabaseDelegate() = default;
  virtual ~BLEDatabaseDelegate() = default;

  virtual void bleDatabaseDidCreate(const BLEDevice& device) = 0;
  
  virtual void bleDatabaseDidUpdate(const BLEDevice& device, const BLEDeviceAttribute attribute) = 0;
  
  virtual void bleDatabaseDidDelete(const BLEDevice& device) = 0;
};

/// \brief List of BLEDatabaseDelegate references
using BLEDatabaseDelegateList = ReferenceArray<BLEDatabaseDelegate,4,false>;

} // end namespace
} // end namespace

#endif