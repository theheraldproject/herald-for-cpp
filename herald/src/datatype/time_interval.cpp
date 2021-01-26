//  Copyright 2020-2021 Herald Project Contributors
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

TimeInterval::TimeInterval(const TimeInterval& other)
  : mImpl(std::make_unique<Impl>())
{
  mImpl->seconds = other.mImpl->seconds;
}

TimeInterval::~TimeInterval() {}


TimeInterval&
TimeInterval::operator=(const TimeInterval& other) noexcept
{
  mImpl->seconds = other.mImpl->seconds;
  return *this;
}

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

TimeInterval::operator long() const noexcept {
  return millis();
}

  
bool
TimeInterval::operator>(const TimeInterval& other) const noexcept
{
  return mImpl->seconds > other.mImpl->seconds;
}

bool
TimeInterval::operator>=(const TimeInterval& other) const noexcept
{
  return mImpl->seconds >= other.mImpl->seconds;
}

bool
TimeInterval::operator<(const TimeInterval& other) const noexcept
{
  return mImpl->seconds < other.mImpl->seconds;
}

bool
TimeInterval::operator<=(const TimeInterval& other) const noexcept
{
  return mImpl->seconds <= other.mImpl->seconds;
}

bool
TimeInterval::operator==(const TimeInterval& other) const noexcept
{
  return mImpl->seconds == other.mImpl->seconds;
}

bool
TimeInterval::operator!=(const TimeInterval& other) const noexcept
{
  return mImpl->seconds != other.mImpl->seconds;
}

long
TimeInterval::millis() const noexcept {
  if (LONG_MAX == mImpl->seconds) {
    return LONG_MAX;
  }
  return mImpl->seconds * 1000;
}

TimeInterval::operator std::string() const noexcept {
  if (mImpl->seconds == LONG_MAX) {
    return "never";
  }
  return std::to_string(mImpl->seconds);
}


} // end namespace
} // end namespace
