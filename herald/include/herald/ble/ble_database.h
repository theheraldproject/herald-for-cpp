//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DATABASE_H
#define BLE_DATABASE_H

#include "ble_database_delegate.h"
#include "ble_device.h"

#include "../datatype/payload_data.h"
#include "../datatype/payload_sharing_data.h"
#include "../datatype/rssi.h"
#include "../datatype/target_identifier.h"

#include <memory>
#include <vector>
#include <functional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class BLEDatabase {
public:
  BLEDatabase() = default;
  virtual ~BLEDatabase() = default;

  virtual void add(const std::shared_ptr<BLEDatabaseDelegate>& delegate) = 0;

  virtual std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac, const Data& advert/*, const RSSI& rssi*/) = 0;

  virtual std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac, const BLEMacAddress& pseudo) = 0;
  
  virtual std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac) = 0;

  virtual std::shared_ptr<BLEDevice> device(const PayloadData& payloadData) = 0;

  virtual std::shared_ptr<BLEDevice> device(const TargetIdentifier& targetIdentifier) = 0;

  // virtual std::vector<std::shared_ptr<BLEDevice>> devices() const = 0;

  virtual std::size_t size() const = 0;

  virtual std::vector<std::shared_ptr<BLEDevice>> matches(
    const std::function<bool(const std::shared_ptr<BLEDevice>&)>& matcher) const = 0;

  /// Cannot name a function delete in C++. remove is common.
  virtual void remove(const TargetIdentifier& targetIdentifier) = 0;

  // virtual PayloadSharingData payloadSharingData(const std::shared_ptr<BLEDevice>& peer) = 0;
};

} // end namespace
} // end namespace

#endif