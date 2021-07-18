//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SENSOR_ARRAY_H
#define HERALD_SENSOR_ARRAY_H

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
#include <array>
#include <variant>
#include <functional>

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
template <typename ContextT, typename PayloadDataSupplierT, typename... SensorTs>
class SensorArray {
public:
  static constexpr std::size_t Size = sizeof...(SensorTs);

  /// \brief Takes ownership of payloadDataSupplier (std::move)
  SensorArray(ContextT& ctx, PayloadDataSupplierT& payloadDataSupplier, SensorTs&... sensors)
  : mContext(ctx), 
    mPayloadDataSupplier(payloadDataSupplier),
    mSensorArray(),
    engine(ctx),
    deviceDescription("")
    HLOGGERINIT(mContext, "Sensor", "SensorArray")
  {
    addSensors(0,sensors...);
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

  PayloadData payloadData() {
    return mPayloadDataSupplier.payload(PayloadTimestamp());
  }

  /// \brief Adds a new sensor to the array, and add its coordination provider to the engine
  // void add(Sensor& sensor) {
  //   mSensorArray.emplace_back(sensor); // adds in links to BLE transmitter, receiver
  //   engine.add(sensor);
  // }

  // SENSOR OVERRIDES
  void start() {
    for (auto& sensor: mSensorArray) {
      sensor.get().start();
    }
    engine.start();
  }

  void stop() {
    engine.stop();
    for (auto& sensor: mSensorArray) {
      sensor.get().stop();
    }
  }

  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() {
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
  PayloadDataSupplierT& mPayloadDataSupplier;
  // std::vector<std::reference_wrapper<Sensor>> mSensorArray;
  std::array<std::variant<std::reference_wrapper<SensorTs...>>,Size> mSensorArray;

  Coordinator<ContextT> engine;

  // Not initialised (and thus optional):-
  std::string deviceDescription;

  HLOGGER(ContextT);

  template <typename LastT>
  constexpr void addSensors(int nextPos,LastT& last) {
    mSensorArray[nextPos] = std::reference_wrapper<LastT>(last);
  }

  template <typename FirstT, typename SecondT, typename... RestT>
  constexpr void addSensors(int nextPos,FirstT& first, SecondT& second, RestT&... rest) {
    mSensorArray[nextPos] = std::reference_wrapper<FirstT>(first);
    ++nextPos;
    addSensors(nextPos,second,rest...);
  }
};




} // end namespace

#endif