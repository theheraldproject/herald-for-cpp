//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_database.h"
#include "herald/ble/ble_database_delegate.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_device.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/data/sensor_logger.h"

// C++17 includes
#include <memory>
#include <vector>
#include <algorithm>

namespace herald {
namespace ble {

using namespace herald::datatype;

class ConcreteBLEDatabase::Impl {
public:
  Impl(std::shared_ptr<Context> context);
  ~Impl();

  std::shared_ptr<Context> ctx;
  std::vector<std::shared_ptr<BLEDatabaseDelegate>> delegates;
  std::vector<std::shared_ptr<BLEDevice>> devices;

  HLOGGER;
};

ConcreteBLEDatabase::Impl::Impl(std::shared_ptr<Context> context) 
  : ctx(context),
    delegates(),
    devices()
    HLOGGERINIT(ctx,"herald","ConcreteBLEDatabase")
{
  ;
}

ConcreteBLEDatabase::Impl::~Impl()
{
  ;
}






ConcreteBLEDatabase::ConcreteBLEDatabase(std::shared_ptr<Context> context)
  : mImpl(std::make_unique<Impl>(context))
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
  auto results = matches([&payloadData](const std::shared_ptr<BLEDevice>& d) {
    auto payload = d->payloadData();
    if (!payload.has_value()) {
      return false;
    }
    return (*payload)==payloadData;
  });
  if (results.size() != 0) {
    return results.front(); // TODO ensure we send back the latest, not just the first match
  }
  std::shared_ptr<BLEDevice> newDevice = std::make_shared<BLEDevice>(
    TargetIdentifier(payloadData), shared_from_this());
  mImpl->devices.push_back(newDevice);
  for (auto delegate : mImpl->delegates) {
    delegate->bleDatabaseDidCreate(newDevice);
  }
  return newDevice;
}

std::shared_ptr<BLEDevice>
ConcreteBLEDatabase::device(const TargetIdentifier& targetIdentifier)
{
  auto results = matches([&targetIdentifier](const std::shared_ptr<BLEDevice>& d) {
    return d->identifier() == targetIdentifier;
  });
  if (results.size() != 0) {
    return results.front(); // TODO ensure we send back the latest, not just the first match
  }
  HDBG("New target identified: {}",(std::string)targetIdentifier);
  std::shared_ptr<BLEDevice> newDevice = std::make_shared<BLEDevice>(
    targetIdentifier, shared_from_this());
  mImpl->devices.push_back(newDevice);
  for (auto delegate : mImpl->delegates) {
    delegate->bleDatabaseDidCreate(newDevice);
  }
  return newDevice;
}

std::size_t
ConcreteBLEDatabase::size() const
{
  return mImpl->devices.size();
}

std::vector<std::shared_ptr<BLEDevice>>
ConcreteBLEDatabase::matches(
  const std::function<bool(std::shared_ptr<BLEDevice>)>& matcher) const
{
  std::vector<std::shared_ptr<BLEDevice>> results;
  // in the absence of copy_if in C++20... Just copies the pointers not the objects
  for (auto d : mImpl->devices) {
    if (matcher(d)) {
      results.push_back(d);
    }
  }
  return results;
}

/// Cannot name a function delete in C++. remove is common.
void
ConcreteBLEDatabase::remove(const TargetIdentifier& targetIdentifier)
{
  auto found = std::find_if(mImpl->devices.begin(),mImpl->devices.end(),
    [&targetIdentifier](std::shared_ptr<BLEDevice> d) -> bool {
      return d->identifier() == targetIdentifier;
    }
  );
  if (found != mImpl->devices.end()) {
    std::shared_ptr<BLEDevice> toRemove = *found;
    mImpl->devices.erase(found);
    for (auto delegate : mImpl->delegates) {
      delegate->bleDatabaseDidDelete(toRemove);
    }
  }
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
