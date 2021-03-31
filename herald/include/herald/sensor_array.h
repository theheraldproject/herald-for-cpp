//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_ARRAY_H
#define SENSOR_ARRAY_H

#include "data/sensor_logger.h"
#include "sensor_delegate.h"
#include "sensor.h"
#include "context.h"
#include "payload/payload_data_supplier.h"
#include "datatype/payload_timestamp.h"
#include "datatype/data.h"
#include "datatype/target_identifier.h"
#include "datatype/payload_data.h"
#include "ble/ble_concrete.h"
#include "engine/coordinator.h"

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

/// \brief Manages all Sensors and sensor delegates for Herald
///
/// This is the Core logic and runtime class for all of Herald.
/// It is platform independent and Sensor technology independent.
/// On C++ this class does NOT use threads. It is instead up to
/// the platform developer to call the iteration(TimeInterval)
/// function from whichever scheduling or threading tools are
/// available on each platform. In Zephyr RTOS, for example,
/// This is a simple 250ms delay within a special Herald-only
/// Zephyr kernel thread.
template <typename ContextT>
class SensorArray : public Sensor {
public:
  /// \brief Takes ownership of payloadDataSupplier (std::move)
  SensorArray(ContextT& ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mContext(ctx), 
    mPayloadDataSupplier(payloadDataSupplier),
    mSensorArray(),
    engine(ctx),
    deviceDescription("")
    HLOGGERINIT(mContext, "Sensor", "SensorArray")
  {
  }

  ~SensorArray() = default;

  // TODO add immediate send support back in when template mechanism determined
  // SENSOR ARRAY METHODS
  // bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) {
  //   return concrete->immediateSend(data, targetIdentifier);
  // }

  // bool immediateSendAll(Data data) {
  //   return concrete->immediateSendAll(data);
  // }

  std::optional<PayloadData> payloadData() {
    return mPayloadDataSupplier->payload(PayloadTimestamp(),nullptr);
  }

  /// \brief Adds a new sensor to the array, and add its coordination provider to the engine
  void add(const std::shared_ptr<Sensor>& sensor) {
    mSensorArray.push_back(sensor); // adds in links to BLE transmitter, receiver
    engine.add(sensor);
  }

  // SENSOR OVERRIDES 
  void add(const std::shared_ptr<SensorDelegate>& delegate) override {
    for (auto& sensor: mSensorArray) {
      sensor->add(delegate);
    }
  }

  void start() override {
    for (auto& sensor: mSensorArray) {
      sensor->start();
    }
    engine.start();
  }

  void stop() override {
    engine.stop();
    for (auto& sensor: mSensorArray) {
      sensor->stop();
    }
  }

  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() override {
    return {};
  }

  /// \brief Scheduling activities from external OS thread wakes - Since v1.2-beta3
  void iteration(const TimeInterval sinceLastCompleted) {
    // TODO ensure this works for continuous evaluation with minimal overhead or battery
    engine.iteration();
  }

private:
  // Initialised on entry to Impl constructor:-
  ContextT& mContext;
  std::shared_ptr<PayloadDataSupplier> mPayloadDataSupplier;
  std::vector<std::shared_ptr<Sensor>> mSensorArray;

  Coordinator<ContextT> engine;

  // Not initialised (and thus optional):-
  std::string deviceDescription;

  HLOGGER(ContextT);
};




} // end namespace

#endif