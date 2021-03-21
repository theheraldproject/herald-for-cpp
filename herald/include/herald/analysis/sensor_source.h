//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ANALYSIS_SENSOR_SOURCE_H
#define HERALD_ANALYSIS_SENSOR_SOURCE_H

#include "sampling.h"
#include "../default_sensor_delegate.h"
#include "../datatype/rssi.h"

namespace herald {
namespace analysis {

using namespace sampling;

/// \brief Connects the RSSI readings from a SensorDelegate to a source for AnalysisRunner data
template <typename RunnerT>
struct SensorDelegateRSSISource : DefaultSensorDelegate {

  SensorDelegateRSSISource(RunnerT& runner) : runner(runner) {};
  ~SensorDelegateRSSISource() = default;

  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) override {
    if (sensor != SensorType::BLE) return; // guard for BLE RSSI proximity only data
    runner.template newSample<RSSI>(withPayload.hashCode(),Sample<RSSI>(Date(),RSSI(didMeasure.value)));
  }

private:
  RunnerT& runner; // reference to app wide Analysis Runner instance
};

}
}

#endif
