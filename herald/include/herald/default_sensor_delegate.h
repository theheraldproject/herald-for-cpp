//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEFAULT_SENSOR_DELEGATE_H
#define HERALD_DEFAULT_SENSOR_DELEGATE_H

#include "sensor_delegate.h"

namespace herald {

/// \brief Default implementation that provides implementations for each delegate callback.
/// \sa SensorDelegate
class DefaultSensorDelegate {
public:
  DefaultSensorDelegate() noexcept = default;
  ~DefaultSensorDelegate() noexcept = default;

  // Note: Now commented out as SensorDelegateSet dynamically checks for presence of the callback (more efficient)

  // /// Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  // void sensor(SensorType sensor, const TargetIdentifier& didDetect);

  // /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  // void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget);

  // /// Receive written immediate send data from target, e.g. important timing signal.
  // void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget);

  // /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  // void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget);

  // /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  // void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget);

  // /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  // template <typename LocationT>
  // void sensor(SensorType sensor, const Location<LocationT>& didVisit);

  // /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  // void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload);

  // /// Sensor state update
  // void sensor(SensorType sensor, const SensorState& didUpdateState);
};




} // end namespace

#endif