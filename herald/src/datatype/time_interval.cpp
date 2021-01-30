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

TimeInterval
TimeInterval::zero() {
  return TimeInterval(0);
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

TimeInterval&
TimeInterval::operator+=(const TimeInterval& other) noexcept {
  mImpl->seconds += other.mImpl->seconds;
  return *this;
}

TimeInterval
TimeInterval::operator-(const TimeInterval& other) noexcept {
  return TimeInterval(mImpl->seconds - other.mImpl->seconds);
}

TimeInterval&
TimeInterval::operator-=(const TimeInterval& other) noexcept {
  mImpl->seconds -= other.mImpl->seconds;
  return *this;
}

TimeInterval
TimeInterval::operator*(double multiplier) noexcept
{
  auto output = mImpl->seconds * multiplier;
  return TimeInterval(static_cast<decltype(mImpl->seconds)>(output));
}

TimeInterval&
TimeInterval::operator*=(double multiplier) noexcept
{
  auto output = mImpl->seconds * multiplier;
  mImpl->seconds = static_cast<decltype(mImpl->seconds)>(output);
  return *this;
}

TimeInterval
TimeInterval::operator/(double divisor) noexcept
{
  if (0 == divisor) {
    return *this;
  }
  auto output = mImpl->seconds / divisor;
  return TimeInterval(static_cast<decltype(mImpl->seconds)>(output));
}

TimeInterval&
TimeInterval::operator/=(double divisor) noexcept
{
  if (0 == divisor) {
    return *this;
  }
  auto output = mImpl->seconds / divisor;
  mImpl->seconds = static_cast<decltype(mImpl->seconds)>(output);
  return *this;
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

long
TimeInterval::seconds() const noexcept {
  return mImpl->seconds;
}

TimeInterval::operator std::string() const noexcept {
  if (mImpl->seconds == LONG_MAX) {
    return "never";
  }
  return std::to_string(mImpl->seconds);
}


} // end namespace
} // end namespace
