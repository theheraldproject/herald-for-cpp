//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/sensor_array.h"
#include "herald/context.h"
#include "herald/data/sensor_logger.h"
#include "herald/datatype/payload_timestamp.h"
#include "herald/payload/payload_data_supplier.h"
#include "herald/ble/ble_concrete.h"
#include "herald/engine/coordinator.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace herald {

using namespace ble;
using namespace data;
using namespace datatype;
using namespace payload;
using namespace engine;

class SensorArray::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ~Impl();

  // Initialised on entry to Impl constructor:-
  std::shared_ptr<Context> mContext;
  std::shared_ptr<PayloadDataSupplier> mPayloadDataSupplier;
  std::vector<std::shared_ptr<Sensor>> mSensorArray;

  std::shared_ptr<ConcreteBLESensor> concrete;

  Coordinator engine;

  // Not initialised (and thus optional):-
  std::string deviceDescription;

  HLOGGER;
};

SensorArray::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mContext(ctx), 
    mPayloadDataSupplier(payloadDataSupplier),
    mSensorArray(),
    concrete(std::make_shared<ConcreteBLESensor>(mContext, mContext->getBluetoothStateManager(),
      mPayloadDataSupplier)),
    engine(ctx),
    deviceDescription("")
    HLOGGERINIT(mContext, "Sensor", "SensorArray")
{
  // PayloadTimestamp pts; // now
  // mPayloadData = mPayloadDataSupplier->payload(pts);
  // add(std::make_shared<ContactLog>(mContext, "contacts.csv"));
  // add(std::make_shared<StatisticsLog>(mContext, "statistics.csv", payloadData));
  // add(std::make_shared<StatisticsDidReadLog>(mContext, "statistics_didRead.csv", payloadData));
  // add(std::make_shared<DetectionLog>(mContext,"detection.csv", payloadData));
  // mBatteryLog = std::make_shared<BatteryLog>(mContext, "battery.csv");

  mSensorArray.push_back(concrete); // adds in links to BLE transmitter, receiver
  engine.add(concrete);

  // deviceDescription = ""; // TODO get the real device description

  // NOTE THE FOLLOWING LINE CAUSES ZEPHYR APPS TO NOT EXECUTE - COUT ISSUE?
  // TODO test this now logging on zephyr is reliable
  //mLogger.info("DEVICE (payload={},description={})", "nil", deviceDescription);
}

SensorArray::Impl::~Impl()
{
  ;
}






/// Takes ownership of payloadDataSupplier (std::move)
SensorArray::SensorArray(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mImpl(std::make_unique<Impl>(ctx,payloadDataSupplier))
{
  ;
}

SensorArray::~SensorArray()
{
  ;
}

// SENSOR ARRAY METHODS
bool
SensorArray::immediateSend(Data data, const TargetIdentifier& targetIdentifier) {
  return mImpl->concrete->immediateSend(data, targetIdentifier);
}

bool
SensorArray::immediateSendAll(Data data) {
  return mImpl->concrete->immediateSendAll(data);
}

std::optional<PayloadData>
SensorArray::payloadData() {
  return mImpl->mPayloadDataSupplier->payload(PayloadTimestamp(),nullptr);
}

// SENSOR OVERRIDES 
void
SensorArray::add(std::shared_ptr<SensorDelegate> delegate) {
  for (auto& sensor: mImpl->mSensorArray) {
    sensor->add(delegate);
  }
}

void
SensorArray::start() {
  for (auto& sensor: mImpl->mSensorArray) {
    sensor->start();
  }
  mImpl->engine.start();
}

void
SensorArray::stop() {
  mImpl->engine.stop();
  for (auto& sensor: mImpl->mSensorArray) {
    sensor->stop();
  }
}

std::optional<std::shared_ptr<CoordinationProvider>>
SensorArray::coordinationProvider()
{
  return {};
}

// Periodic actions
void
SensorArray::iteration(const TimeInterval sinceLastCompleted)
{
  // TODO ensure this works for continuous evaluation with minimal overhead or battery
  mImpl->engine.iteration();
}


} // end namespace
