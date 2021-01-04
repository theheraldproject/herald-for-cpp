//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/location.h"
#include "herald/datatype/location_reference.h"
#include "herald/datatype/date.h"

#include <string>

namespace herald {
namespace datatype {

// IMPL DEFINITION

class Location::Impl {
public:
  Impl(std::shared_ptr<LocationReference> value, Date start, Date end);
  ~Impl() = default;

  // member variables
  std::shared_ptr<LocationReference> mValue;
  Date mStart;
  Date mEnd;
};

// IMPL DECLARATIONS
Location::Impl::Impl(std::shared_ptr<LocationReference> value, Date start, Date end)
  : mValue(value),
    mStart(std::move(start)),
    mEnd(std::move(end))
{
  ;
}




// LOCATION DECLARATIONS

Location::Location(std::shared_ptr<LocationReference> value, Date start, Date end)
  : mImpl(std::make_unique<Impl>(value,start,end))
{
  ;
}

std::string
Location::description() const {
  return mImpl->mValue->description() + ":[from=" + ((std::string)mImpl->mStart) + ",to=" + ((std::string)mImpl->mEnd) + "]";
}

Location::~Location() {}


} // end namespace
} // end namespace
