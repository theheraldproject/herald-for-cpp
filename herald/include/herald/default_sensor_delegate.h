//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DEFAULT_SENSOR_DELEGATE_H
#define DEFAULT_SENSOR_DELEGATE_H

#include "sensor_delegate.h"

#include <vector>

namespace herald {

class DefaultSensorDelegate : public SensorDelegate {
public:
  DefaultSensorDelegate();
  ~DefaultSensorDelegate() = default;
  
  /// Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) override;

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, PayloadData didRead, const TargetIdentifier& fromTarget) override;

  /// Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, ImmediateSendData didReceive, const TargetIdentifier& fromTarget) override;

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, std::vector<PayloadData> didShare, const TargetIdentifier& fromTarget) override;

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget) override;

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  void sensor(SensorType sensor, Location didVisit) override;

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget, PayloadData withPayload) override;

  /// Sensor state update
  void sensor(SensorType sensor, SensorState didUpdateState) override;
};




} // end namespace

#endif