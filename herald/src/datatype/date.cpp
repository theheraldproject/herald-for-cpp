//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/date.h"

#include <chrono>
#include <ctime>

namespace herald {
namespace datatype {

// PIMPL DEFINITION
class Date::Impl {
public:
  Impl(); // now
  Impl(long secondsSinceUnixEpoch);
  ~Impl() = default;

  long seconds;
};

// PIMPL DECLARATIONS
Date::Impl::Impl()
 : seconds(std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count())
{
  ;
}

Date::Impl::Impl(long secondsSinceUnixEpoch)
 : seconds(secondsSinceUnixEpoch)
{
  ;
}

// DATE DECLARATIONS

Date::Date()
 : mImpl(std::make_unique<Impl>())
{
  ;
}

Date::Date(long secondsSinceEpoch)
 : mImpl(std::make_unique<Impl>(secondsSinceEpoch))
{
  ;
}

Date::Date(const Date& from)
 : mImpl(std::make_unique<Impl>(from.mImpl->seconds))
{
  ;
}

Date::~Date() {}

Date&
Date::operator=(const Date& other) noexcept
{
  mImpl->seconds = other.mImpl->seconds;
  return *this;
}

std::string
Date::iso8601DateTime() const noexcept {
  time_t t(mImpl->seconds);
  char buf[21];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));
  return std::string(buf);
}

Date::operator std::string() const noexcept {
  return iso8601DateTime();
}

long
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
