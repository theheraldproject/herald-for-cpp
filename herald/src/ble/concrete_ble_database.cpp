//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_database.h"
#include "herald/ble/ble_database_delegate.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_device.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/bluetooth_state.h"

// C++17 includes
#include <memory>
#include <vector>

namespace herald {
namespace ble {

using namespace herald::datatype;


// TODO functional filter classes as required - consider moving to the header for re-use
// class with_identifier {
//   bool operator()
// };

// class with_payload {

// };


class ConcreteBLEDatabase::Impl {
public:
  Impl();
  ~Impl();

  std::vector<std::shared_ptr<BLEDatabaseDelegate>> delegates;
  std::vector<std::shared_ptr<BLEDevice>> devices;
};

ConcreteBLEDatabase::Impl::Impl() 
  : delegates(),
    devices()
{
  ;
}

ConcreteBLEDatabase::Impl::~Impl()
{
  ;
}






ConcreteBLEDatabase::ConcreteBLEDatabase()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

ConcreteBLEDatabase::~ConcreteBLEDatabase()
{
  ;
}

// BLE Database overrides

void
ConcreteBLEDatabase::add(const std::shared_ptr<BLEDatabaseDelegate>& delegate)
{
  mImpl->delegates.push_back(delegate);
}

std::shared_ptr<BLEDevice>
ConcreteBLEDatabase::device(const PayloadData& payloadData)
{
  // TODO proper implementation
  return mImpl->devices.front();
}

std::shared_ptr<BLEDevice>
ConcreteBLEDatabase::device(const TargetIdentifier& targetIdentifier)
{
  // TODO proper implementation
  return mImpl->devices.front();
}

std::vector<std::shared_ptr<BLEDevice>>
ConcreteBLEDatabase::devices() const
{
  return mImpl->devices;
}

/// Cannot name a function delete in C++. remove is common.
void
ConcreteBLEDatabase::remove(const TargetIdentifier& targetIdentifier)
{
  // TODO fill out this function
}

std::optional<PayloadSharingData>
ConcreteBLEDatabase::payloadSharingData(const std::shared_ptr<BLEDevice>& peer)
{
  return std::optional<PayloadSharingData>(); // TODO look this up
}

// BLE Device Delegate overrides
void
ConcreteBLEDatabase::device(std::shared_ptr<BLEDevice> device, const BLEDeviceAttribute didUpdate)
{
  // TODO update any internal DB state as necessary (E.g. deletion)
  for (auto delegate : mImpl->delegates) {
    delegate->bleDatabaseDidUpdate(device, didUpdate); // TODO verify this is the right onward call
  }
}

}
}
