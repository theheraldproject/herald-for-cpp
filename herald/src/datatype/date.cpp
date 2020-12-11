//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "datatype/date.h"

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
 : seconds(0) // TODO FIX THIS FOR EACH PLATFORM WE ARE ON
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

std::string
Date::iso8601DateTime() const {
  return ""; // TODO fix this
}

std::string
Date::toString() const {
  return iso8601DateTime();
}

long
Date::secondsSinceUnixEpoch() const {
  return mImpl->seconds;
}


} // end namespace
} // end namespace
