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

#include <ctime>

namespace herald {
namespace datatype {

// PIMPL DEFINITION
class Date::Impl {
public:
  Impl(); // now
  Impl(std::uint64_t secondsSinceEpochOrRestart);
  ~Impl() = default;

  std::uint64_t seconds;
};

// PIMPL DECLARATIONS
Date::Impl::Impl()
 : 
#ifdef __ZEPHYR__
  seconds(0.001 * k_uptime_get())
#else
 seconds(std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count())
#endif
{
  ;
}

Date::Impl::Impl(std::uint64_t secondsSinceEpochOrRestart)
 : seconds(secondsSinceEpochOrRestart)
{
  ;
}

// DATE DECLARATIONS

Date::Date()
 : mImpl(std::make_unique<Impl>())
{
  ;
}

Date::Date(std::uint64_t secondsSinceEpochOrRestart)
 : mImpl(std::make_unique<Impl>(secondsSinceEpochOrRestart))
{
  ;
}

Date::Date(const Date& from)
 : mImpl(std::make_unique<Impl>(from.mImpl->seconds))
{
  ;
}

Date::~Date() = default;

Date&
Date::operator=(const Date& other) noexcept
{
  mImpl->seconds = other.mImpl->seconds;
  return *this;
}

Date
Date::operator-(const TimeInterval& other) noexcept
{
  return Date(mImpl->seconds - other.seconds());
}

Date&
Date::operator-=(const TimeInterval& other) noexcept
{
  mImpl->seconds -= other.seconds();
  return *this;
}

Date
Date::operator+(const TimeInterval& other) noexcept
{
  return Date(mImpl->seconds + other.seconds());
}

Date&
Date::operator+=(const TimeInterval& other) noexcept
{
  mImpl->seconds += other.seconds();
  return *this;
}

std::string
Date::iso8601DateTime() const noexcept {
  time_t t(mImpl->seconds);
  char buf[21];
  strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&t));
  return std::string(buf);
}

Date::operator std::string() const noexcept {
  return iso8601DateTime();
}

std::uint64_t
Date::secondsSinceUnixEpoch() const noexcept {
  return mImpl->seconds;
}

bool
Date::operator==(const Date& other) const noexcept
{
  return mImpl->seconds == other.mImpl->seconds;
}

bool
Date::operator!=(const Date& other) const noexcept
{
  return mImpl->seconds != other.mImpl->seconds;
}

bool
Date::operator<(const Date& other) const noexcept
{
  return mImpl->seconds < other.mImpl->seconds;
}

bool
Date::operator>(const Date& other) const noexcept
{
  return mImpl->seconds > other.mImpl->seconds;
}

bool
Date::operator<=(const Date& other) const noexcept
{
  return mImpl->seconds <= other.mImpl->seconds;
}

bool
Date::operator>=(const Date& other) const noexcept
{
  return mImpl->seconds >= other.mImpl->seconds;
}


Date::operator long() const noexcept
{
  return mImpl->seconds;
}


} // end namespace
} // end namespace
