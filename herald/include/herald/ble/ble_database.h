//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DATABASE_H
#define BLE_DATABASE_H

#include "ble_database_delegate.h"
#include "ble_device.h"

#include "../datatype/payload_data.h"
#include "../datatype/payload_sharing_data.h"
#include "../datatype/target_identifier.h"

#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class BLEDatabase {
public:
  BLEDatabase() = default;
  virtual ~BLEDatabase() = default;

  virtual void add(const std::shared_ptr<BLEDatabaseDelegate>& delegate) = 0;

  //virtual std::shared_ptr<BLEDevice> device(const ScanResult& scanResult) = 0;

  //virtual std::shared_ptr<BLEDevice> device(const BluetoothDevice& bluetoothDevice) = 0;

  virtual std::shared_ptr<BLEDevice> device(const PayloadData& payloadData) = 0;

  virtual std::shared_ptr<BLEDevice> device(const TargetIdentifier& targetIdentifier) = 0;

  virtual std::vector<std::shared_ptr<BLEDevice>> devices() const = 0;

  /// Cannot name a function delete in C++. remove is common.
  virtual void remove(const TargetIdentifier& targetIdentifier) = 0;

  virtual std::optional<PayloadSharingData> payloadSharingData(const std::shared_ptr<BLEDevice>& peer) = 0;
};

} // end namespace
} // end namespace

#endif