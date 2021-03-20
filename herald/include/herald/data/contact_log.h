//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef CONTACT_LOG_H
#define CONTACT_LOG_H

#include "../sensor_delegate.h"
#include "payload_data_formatter.h"
#include "sensor_logger.h"

namespace herald::data {

/**
 * Logs all contact info to STDERR to allow it to be extracted as a stream
 */
class ErrorStreamContactLogger : public SensorDelegate {
public:
  ErrorStreamContactLogger(std::shared_ptr<Context> context, std::shared_ptr<PayloadDataFormatter> formatter);
  ~ErrorStreamContactLogger();

  // Sensor delegate overrides
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) override;
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) override;
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) override;
  void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) override;
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) override;
  template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit);
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) override;
  void sensor(SensorType sensor, const SensorState& didUpdateState) override;

private:
  class Impl; // fwd decl
  std::unique_ptr<Impl> mImpl; // PIMPL idiom
};

}

#endif