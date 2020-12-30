//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/context.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/ble_transmitter.h"
#include "herald/ble/bluetooth_state_manager.h"

#include <memory>
#include <vector>

namespace herald {
namespace ble {

using namespace herald::datatype;

// PIMPL class

class ConcreteBLESensor::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ~Impl();

  std::shared_ptr<Context> mContext;
  std::shared_ptr<PayloadDataSupplier> mPayloadDataSupplier;

  std::vector<std::shared_ptr<SensorDelegate>> mDelegates;
};

ConcreteBLESensor::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mContext(ctx),
    mPayloadDataSupplier(payloadDataSupplier),
    mDelegates()
{
  ;
}

ConcreteBLESensor::Impl::~Impl() {
  ;
}



// Concrete class

ConcreteBLESensor::ConcreteBLESensor(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier) 
  : mImpl(std::make_unique<Impl>(ctx, payloadDataSupplier))
{
  ;
}

ConcreteBLESensor::~ConcreteBLESensor() {
  ;
}

bool
ConcreteBLESensor::immediateSend(Data data, const TargetIdentifier& targetIdentifier) {
  return true; // TODO implement this function
}

// TODO BLESensor overrides


void
ConcreteBLESensor::add(std::shared_ptr<SensorDelegate> delegate) {
  mImpl->mDelegates.push_back(delegate);
  // TODO alert sub-sensors to new delegate
}

void
ConcreteBLESensor::start() {
  // TODO start sub sensors
}

void
ConcreteBLESensor::stop() {
  // TODO stop sub sensors
}

} // end namespace
} // end namespace