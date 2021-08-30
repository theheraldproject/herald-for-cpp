//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/time_interval.h"

namespace herald {
namespace datatype {

// class TimeInterval::Impl {
// public:
//   Impl();
//   Impl(long secondsSinceUnixEpoch);
//   ~Impl() = default;

//   long seconds;
// };

// TimeInterval::Impl::Impl() : seconds(0) { }

// TimeInterval::Impl::Impl(long secondsSinceUnixEpoch) : seconds(secondsSinceUnixEpoch) { }





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
 : secs(seconds)
{
  ;
}

TimeInterval::TimeInterval(const Date& date)
 : secs(date.secondsSinceUnixEpoch())
{
  ;
}

TimeInterval::TimeInterval(const Date& from, const Date& to)
 : secs(to.secondsSinceUnixEpoch() - from.secondsSinceUnixEpoch())
{
  ;
}

TimeInterval::TimeInterval(const TimeInterval& other)
  : secs(other.secs)
{
  ;
}

TimeInterval::~TimeInterval() {}


TimeInterval&
TimeInterval::operator=(const TimeInterval& other) noexcept
{
  secs = other.secs;
  return *this;
}

TimeInterval
TimeInterval::operator*(const TimeInterval& other) noexcept {
  return TimeInterval(secs * other.secs);
}

TimeInterval
TimeInterval::operator+(const TimeInterval& other) noexcept {
  return TimeInterval(secs + other.secs);
}

TimeInterval&
TimeInterval::operator+=(const TimeInterval& other) noexcept {
  secs += other.secs;
  return *this;
}

TimeInterval
TimeInterval::operator-(const TimeInterval& other) noexcept {
  return TimeInterval(secs - other.secs);
}

TimeInterval&
TimeInterval::operator-=(const TimeInterval& other) noexcept {
  secs -= other.secs;
  return *this;
}

TimeInterval
TimeInterval::operator*(double multiplier) noexcept
{
  auto output = secs * multiplier;
  return TimeInterval(static_cast<decltype(secs)>(output));
}

TimeInterval&
TimeInterval::operator*=(double multiplier) noexcept
{
  auto output = secs * multiplier;
  secs = static_cast<decltype(secs)>(output);
  return *this;
}

TimeInterval
TimeInterval::operator/(double divisor) noexcept
{
  if (0 == divisor) {
    return *this;
  }
  auto output = secs / divisor;
  return TimeInterval(static_cast<decltype(secs)>(output));
}

TimeInterval&
TimeInterval::operator/=(double divisor) noexcept
{
  if (0 == divisor) {
    return *this;
  }
  auto output = secs / divisor;
  secs = static_cast<decltype(secs)>(output);
  return *this;
}

TimeInterval::operator long() const noexcept {
  return millis();
}

  
bool
TimeInterval::operator>(const TimeInterval& other) const noexcept
{
  return secs > other.secs;
}

bool
TimeInterval::operator>=(const TimeInterval& other) const noexcept
{
  return secs >= other.secs;
}

bool
TimeInterval::operator<(const TimeInterval& other) const noexcept
{
  return secs < other.secs;
}

bool
TimeInterval::operator<=(const TimeInterval& other) const noexcept
{
  return secs <= other.secs;
}

bool
TimeInterval::operator==(const TimeInterval& other) const noexcept
{
  return secs == other.secs;
}

bool
TimeInterval::operator!=(const TimeInterval& other) const noexcept
{
  return secs != other.secs;
}

long
TimeInterval::millis() const noexcept {
  if (LONG_MAX == secs) {
    return LONG_MAX;
  }
  return secs * 1000;
}

long
TimeInterval::seconds() const noexcept {
  return secs;
}

TimeInterval::operator std::string() const noexcept {
  if (secs == LONG_MAX) {
    return "never";
  }
  return std::to_string(secs);
}


} // end namespace
} // end namespace
