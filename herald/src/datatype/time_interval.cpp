//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/time_interval.h"

namespace herald {
namespace datatype {

class TimeInterval::Impl {
public:
  Impl();
  Impl(long secondsSinceUnixEpoch);
  ~Impl() = default;

  long seconds;
};

TimeInterval::Impl::Impl() : seconds(0) { }

TimeInterval::Impl::Impl(long secondsSinceUnixEpoch) : seconds(secondsSinceUnixEpoch) { }





TimeInterval
TimeInterval::minutes(long minutes) {
  return TimeInterval(60 * minutes);
}

TimeInterval
TimeInterval::seconds(long seconds) {
  return TimeInterval(seconds);
}

TimeInterval
TimeInterval::never() {
  return TimeInterval(LONG_MAX);
}


TimeInterval::TimeInterval(long seconds)
 : mImpl(std::make_unique<Impl>(seconds))
{
  ;
}

TimeInterval::TimeInterval(const Date& date)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->seconds = date.secondsSinceUnixEpoch();
}

TimeInterval::TimeInterval(const Date& from, const Date& to)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->seconds = to.secondsSinceUnixEpoch() - from.secondsSinceUnixEpoch();
}

TimeInterval::~TimeInterval() {}


TimeInterval
TimeInterval::operator*(const TimeInterval& other) noexcept {
  return TimeInterval(mImpl->seconds * other.mImpl->seconds);
}

TimeInterval
TimeInterval::operator+(const TimeInterval& other) noexcept {
  return TimeInterval(mImpl->seconds + other.mImpl->seconds);
}

TimeInterval
TimeInterval::operator-(const TimeInterval& other) noexcept {
  return TimeInterval(mImpl->seconds - other.mImpl->seconds);
}

long
TimeInterval::operator()() noexcept {
  return millis();
}

long
TimeInterval::millis() {
  if (LONG_MAX == mImpl->seconds) {
    return LONG_MAX;
  }
  return mImpl->seconds * 1000;
}

std::string
TimeInterval::toString() {
  if (mImpl->seconds == never().mImpl->seconds) {
    return "never";
  }
  return std::to_string(mImpl->seconds);
}


} // end namespace
} // end namespace
