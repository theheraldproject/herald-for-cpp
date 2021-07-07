//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SENSOR_DELEGATE_H
#define HERALD_SENSOR_DELEGATE_H

#include "datatype/sensor_type.h"
#include "datatype/proximity.h"
#include "datatype/payload_data.h"
#include "datatype/target_identifier.h"
#include "datatype/immediate_send_data.h"
#include "datatype/location.h"
#include "datatype/sensor_state.h"

#include <vector>

namespace herald {
  
using namespace datatype;

/// \brief Base interface for classes wishing to implement callbacks for core low-level Herald proximity and presence events.
class SensorDelegate {
public:
  SensorDelegate() = default;
  virtual ~SensorDelegate() = default;

  
  /// Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  virtual void sensor(SensorType sensor, const TargetIdentifier& didDetect) = 0;

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  virtual void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) = 0;

  /// Receive written immediate send data from target, e.g. important timing signal.
  virtual void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) = 0;

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  virtual void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) = 0;

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  virtual void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) = 0;

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) {}

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  virtual void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) = 0;

  /// Sensor state update
  virtual void sensor(SensorType sensor, const SensorState& didUpdateState) = 0;
};




} // end namespace

#endif