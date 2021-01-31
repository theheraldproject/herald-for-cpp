//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/data/contact_log.h"
#include "herald/data/sensor_logger.h"
#include "herald/datatype/payload_data.h"
#include "herald/datatype/target_identifier.h"

#include <string>

namespace herald {
namespace data {

using namespace herald::datatype;

class ErrorStreamContactLogger::Impl {
public:
  Impl(std::shared_ptr<Context> context, std::shared_ptr<PayloadDataFormatter> formatter);
  ~Impl();

  std::string csv(std::string toEscape) const noexcept;
  std::string timestamp() const noexcept;

  std::shared_ptr<Context> ctx;
  std::shared_ptr<PayloadDataFormatter> fmt;

  HLOGGER;
};

ErrorStreamContactLogger::Impl::Impl(std::shared_ptr<Context> context, std::shared_ptr<PayloadDataFormatter> formatter)
  : ctx(context), 
    fmt(formatter)
    HLOGGERINIT(ctx,"Sensor","contacts.log")
{
  // logger.fault("ctor test");
  ;
}

ErrorStreamContactLogger::Impl::~Impl() = default;

std::string
ErrorStreamContactLogger::Impl::csv(std::string toEscape) const noexcept
{
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

std::string
ErrorStreamContactLogger::Impl::timestamp() const noexcept
{
  return Date().iso8601DateTime();
}





ErrorStreamContactLogger::ErrorStreamContactLogger(std::shared_ptr<Context> context, std::shared_ptr<PayloadDataFormatter> formatter)
  : mImpl(std::make_unique<Impl>(context, formatter))
{
  ;
}


ErrorStreamContactLogger::~ErrorStreamContactLogger() = default;

// Sensor delegate overrides
void
ErrorStreamContactLogger::sensor(SensorType sensor, const TargetIdentifier& didDetect)
{
  HDBG("didDetect");
  // HDBG("flibble");
  // std::string s = mImpl->timestamp();
  // s += ",";
  // s += str(sensor);
  // s += ",";
  // s += mImpl->csv((std::string)didDetect);
  // s += ",1,,,,,";
  // HERR(s);
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget)
{
  // std::string s = mImpl->timestamp();
  // s += ",";
  // s += str(sensor);
  // s += ",";
  // s += mImpl->csv((std::string)fromTarget);
  // s += ",,2,,,,";
  // s += mImpl->csv(mImpl->fmt->shortFormat(didRead));

  // HERR(s);
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget)
{
  ;
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget)
{
  // std::string s = mImpl->timestamp();
  // s += ",";
  // s += str(sensor);
  // s += ",";
  // s += mImpl->csv((std::string)fromTarget);
  // s += ",,,,4,,";
  // // std::string prefix = mImpl->timestamp() + "," + str(sensor) + "," + mImpl->csv((std::string)fromTarget) + ",,,,4,,";
  // for (auto& payload : didShare) {
  //   std::string final = s; // copy
  //   final += mImpl->csv(mImpl->fmt->shortFormat(payload));
  //   HERR(final);
  //   // HERR(prefix + mImpl->csv(mImpl->fmt->shortFormat(payload)));
  // }
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget)
{
  // std::string s = mImpl->timestamp();
  // s += ",";
  // s += str(sensor);
  // s += ",";
  // s += mImpl->csv((std::string)fromTarget);
  // s += ",,,3,,,";
  // s += mImpl->csv((std::string)didMeasure);
  // HERR(s);

  // HERR(mImpl->timestamp() + "," + str(sensor) + "," + mImpl->csv((std::string)fromTarget) + ",,,3,,," + mImpl->csv((std::string)didMeasure));
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const Location& didVisit)
{
  // std::string s = mImpl->timestamp();
  // s += ",";
  // s += str(sensor);
  // s += ",,,,,,5,";
  // s += mImpl->csv((std::string)didVisit);
  // HERR(s);

  // HERR(mImpl->timestamp() + "," + str(sensor) + ",,,,,,5," + mImpl->csv((std::string)didVisit));
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload)
{
  ;
}

void
ErrorStreamContactLogger::sensor(SensorType sensor, const SensorState& didUpdateState)
{
  ;
}
  
}
}