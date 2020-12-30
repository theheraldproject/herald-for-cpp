//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/encounter.h"

#include <string>

namespace herald {
namespace datatype {

// PIMPL DEFINITION
class Encounter::Impl {
public:
  Impl();
  Impl(Proximity didMeasure, PayloadData withPayload, Date timestamp);
  ~Impl() = default;

  Date date;
  Proximity proximity;
  PayloadData payloadData;
  bool valid;
};


// PIMPL DECLARATIONS
Encounter::Impl::Impl()
  : date(), proximity(), payloadData(), valid(false)
{
  ;
}

Encounter::Impl::Impl(Proximity didMeasure, PayloadData withPayload, Date timestamp)
  : date(timestamp), proximity(didMeasure), payloadData(withPayload), valid(true)
{
  ;
}

// ENCOUNTER DECLARATIONS

Encounter::Encounter(Proximity didMeasure, PayloadData withPayload, Date timestamp)
  : mImpl(std::make_unique<Impl>(didMeasure,withPayload,timestamp))
{
  ;
}

Encounter::Encounter(Proximity didMeasure, PayloadData withPayload)
  : mImpl(std::make_unique<Impl>(didMeasure,withPayload,std::move(Date())))
{
  ;
}

Encounter::Encounter(const std::string csvRow)
  : mImpl(std::make_unique<Impl>())
{
  ;
  // TODO parse the csv
}

Encounter::~Encounter() {}


std::string
Encounter::csvString() const {
  return ""; // TODO fill this out properly
}

bool
Encounter::isValid() const {
  return mImpl->valid;
}

const Proximity&
Encounter::proximity() const
{
  return mImpl->proximity;
}

const PayloadData&
Encounter::payload() const
{
  return mImpl->payloadData;
}

const Date&
Encounter::timestamp() const
{
  return mImpl->date;
}

} // end namespace
} // end namespace
