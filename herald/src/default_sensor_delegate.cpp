//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/default_sensor_delegate.h"

namespace herald {

DefaultSensorDelegate::DefaultSensorDelegate() {}
  
void 
DefaultSensorDelegate::sensor(SensorType sensor, const TargetIdentifier& didDetect) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) {
};

void
DefaultSensorDelegate::sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) {
};

template <typename LocationT>
void 
DefaultSensorDelegate::sensor(SensorType sensor, const Location<LocationT>& didVisit) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, const SensorState& didUpdateState) {
};





} // end namespace