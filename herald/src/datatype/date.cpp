//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/date.h"
#include "herald/datatype/time_interval.h"

#ifdef __ZEPHYR__
#include <kernel.h>
#else
#include <chrono>
#endif

// #include <ctime>

namespace herald {
namespace datatype {

// DATE DECLARATIONS

Date::Date()
  :
#ifdef __ZEPHYR__
  seconds(0.001 * k_uptime_get())
#else
  seconds(std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count())
#endif
{
}

Date::Date(std::uint64_t secondsSinceEpochOrRestart)
 : seconds(secondsSinceEpochOrRestart)
{
  ;
}

Date::Date(const Date& from)
 : seconds(from.seconds)
{
  ;
}

Date::Date(Date&& from)
 : seconds(from.seconds)
{
  ;
}

Date::~Date() = default;

void
Date::setToNow() noexcept
{
  #ifdef __ZEPHYR__
    seconds = 0.001 * k_uptime_get();
  #else
    seconds = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  #endif
}

Date&
Date::operator=(const Date& other) noexcept
{
  seconds = other.seconds;
  return *this;
}

Date&
Date::operator=(Date&& other) noexcept
{
  seconds = other.seconds;
  return *this;
}

Date
Date::operator-(const TimeInterval& other) noexcept
{
  return Date(seconds - other.seconds());
}

Date&
Date::operator-=(const TimeInterval& other) noexcept
{
  seconds -= other.seconds();
  return *this;
}

Date
Date::operator+(const TimeInterval& other) noexcept
{
  return Date(seconds + other.seconds());
}

Date&
Date::operator+=(const TimeInterval& other) noexcept
{
  seconds += other.seconds();
  return *this;
}

std::string
Date::iso8601DateTime() const noexcept {
  // time_t t(seconds);
  // char buf[21];
  // strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&t));
  // return std::string(buf);
  return std::to_string(seconds);
}

Date::operator std::string() const noexcept {
  return iso8601DateTime();
}

std::uint64_t
Date::secondsSinceUnixEpoch() const noexcept {
  return seconds;
}

bool
Date::operator==(const Date& other) const noexcept
{
  return seconds == other.seconds;
}

bool
Date::operator!=(const Date& other) const noexcept
{
  return seconds != other.seconds;
}

bool
Date::operator<(const Date& other) const noexcept
{
  return seconds < other.seconds;
}

bool
Date::operator>(const Date& other) const noexcept
{
  return seconds > other.seconds;
}

bool
Date::operator<=(const Date& other) const noexcept
{
  return seconds <= other.seconds;
}

bool
Date::operator>=(const Date& other) const noexcept
{
  return seconds >= other.seconds;
}


Date::operator long() const noexcept
{
  return seconds;
}


} // end namespace
} // end namespace
