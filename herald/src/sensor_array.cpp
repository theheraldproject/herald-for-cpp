//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "sensor_array.h"
#include "context.h"
#include "data/sensor_logger.h"
#include "datatype/payload_timestamp.h"
#include "payload/payload_data_supplier.h"
#include "payload/concrete_payload_data_supplier.h"
#include "ble/ble_concrete.h"

#include <memory>
#include <string>
#include <vector>

namespace herald {

using namespace ble;
using namespace data;
using namespace datatype;
using namespace payload;

class SensorArray::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ~Impl() = default;

  // Initialised on entry to Impl constructor:-
  std::shared_ptr<Context> mContext;
  std::shared_ptr<PayloadDataSupplier> mPayloadDataSupplier;
  std::vector<std::shared_ptr<Sensor>> mSensorArray;
  SensorLogger mLogger;
  std::shared_ptr<ConcreteBLESensor> mConcreteBleSensor;

  // initialised in IMPL constructor:-
  std::shared_ptr<PayloadData> mPayloadData;
  //std::shared_ptr<BatteryLog> mBatteryLog;


  // Not initialised (and thus optional):-
  std::string deviceDescription;
};

SensorArray::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mContext(ctx), 
    mPayloadDataSupplier(payloadDataSupplier),
    mSensorArray(),
    mLogger(mContext, "Sensor", "SensorArray"),
    mConcreteBleSensor(std::make_shared<ConcreteBLESensor>(mContext, mPayloadDataSupplier))
{
  PayloadTimestamp pts; // now
  // mPayloadData = mPayloadDataSupplier->payload(pts);
  // add(std::make_shared<ContactLog>(mContext, "contacts.csv"));
  // add(std::make_shared<StatisticsLog>(mContext, "statistics.csv", payloadData));
  // add(std::make_shared<StatisticsDidReadLog>(mContext, "statistics_didRead.csv", payloadData));
  // add(std::make_shared<DetectionLog>(mContext,"detection.csv", payloadData));
  // mBatteryLog = std::make_shared<BatteryLog>(mContext, "battery.csv");

  deviceDescription = ""; // TODO get the real device description

  mLogger.info("DEVICE (payload={},description={})", mPayloadData->shortName(), deviceDescription);
}



/// Takes ownership of payloadDataSupplier (std::move)
SensorArray::SensorArray(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mImpl(std::make_unique<Impl>(ctx,payloadDataSupplier))
{
  ;
}

// SENSOR ARRAY METHODS
bool
SensorArray::immediateSend(Data data, const TargetIdentifier& targetIdentifier) {
  return mImpl->mConcreteBleSensor->immediateSend(data,targetIdentifier);
}

std::shared_ptr<PayloadData>
SensorArray::payloadData() {
  return mImpl->mPayloadData;
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
}

void
SensorArray::stop() {
  for (auto& sensor: mImpl->mSensorArray) {
    sensor->stop();
  }
}





} // end namespace
