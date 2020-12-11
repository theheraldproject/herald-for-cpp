//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "default_sensor_delegate.h"

namespace herald {

DefaultSensorDelegate::DefaultSensorDelegate() {}
  
void 
DefaultSensorDelegate::sensor(SensorType sensor, const TargetIdentifier& didDetect) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, PayloadData didRead, const TargetIdentifier& fromTarget) {
};

void
DefaultSensorDelegate::sensor(SensorType sensor, ImmediateSendData didReceive, const TargetIdentifier& fromTarget) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, std::vector<PayloadData> didShare, const TargetIdentifier& fromTarget) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, Location didVisit) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget, PayloadData withPayload) {
};

void 
DefaultSensorDelegate::sensor(SensorType sensor, SensorState didUpdateState) {
};





} // end namespace