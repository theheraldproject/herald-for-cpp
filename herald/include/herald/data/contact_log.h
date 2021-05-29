//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_CONTACT_LOG_H
#define HERALD_CONTACT_LOG_H

#include "../sensor_delegate.h"
#include "payload_data_formatter.h"
#include "sensor_logger.h"
#include "../context.h"

namespace herald::data {

/**
 * Logs all contact info to STDERR to allow it to be extracted as a stream
 */
template <typename ContextT>
class ErrorStreamContactLogger : public SensorDelegate {
public:
  ErrorStreamContactLogger(ContextT& context, std::shared_ptr<PayloadDataFormatter> formatter)
    : ctx(context), 
      fmt(formatter)
      HLOGGERINIT(ctx,"Sensor","contacts.log")
  {
    ;
  }
  ~ErrorStreamContactLogger() = default;

  // Sensor delegate overrides
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) override {
    HTDBG("didDetect");
  }
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) override {}
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) override {}
  void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) override{}
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) override {}
  template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) {}
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) override {}
  void sensor(SensorType sensor, const SensorState& didUpdateState) override {}

private:
  std::string csv(std::string toEscape) const noexcept {
    // C++23 only: if (toEscape.contains(",") || toEscape.contains("\"") || toEscape.contains("'") || toEscape.contains("’")) {
    // Pre C++23:-
    if (std::string::npos != toEscape.find(",") || 
        std::string::npos != toEscape.find("\"") || 
        std::string::npos != toEscape.find("'") || 
        std::string::npos != toEscape.find("’")) {
      return "\"" + toEscape + "\"";
    }
    return toEscape;
  }

  std::string timestamp() const noexcept {
    return Date().iso8601DateTime();
  }

  ContextT& ctx;
  std::shared_ptr<PayloadDataFormatter> fmt;

  HLOGGER(ContextT);
};

}

#endif