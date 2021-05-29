//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/encounter.h"

#include <string>

namespace herald {
namespace datatype {

// PIMPL DEFINITION
// class Encounter::Impl {
// public:
//   Impl();
//   Impl(Proximity didMeasure, PayloadData withPayload, Date timestamp);
//   ~Impl() = default;

//   Date date;
//   Proximity proximity;
//   PayloadData payloadData;
//   bool valid;
// };


// // PIMPL DECLARATIONS
// Encounter::Impl::Impl()
//   : date(), proximity(), payloadData(), valid(false)
// {
//   ;
// }

// Encounter::Impl::Impl(Proximity didMeasure, PayloadData withPayload, Date timestamp)
//   : date(timestamp), proximity(didMeasure), payloadData(withPayload), valid(true)
// {
//   ;
// }

// ENCOUNTER DECLARATIONS

Encounter::Encounter(Proximity didMeasure, PayloadData withPayload, Date timestamp)
  : date(timestamp), prox(didMeasure), payloadData(withPayload), valid(true)
{
  ;
}

Encounter::Encounter(Proximity didMeasure, PayloadData withPayload)
  : date(Date()), prox(didMeasure), payloadData(withPayload), valid(true)
{
  ;
}

Encounter::Encounter(const std::string csvRow)
  : date(), prox(), payloadData(), valid(false)
{
  ;
  // TODO parse the csv
}

Encounter::~Encounter() = default;


std::string
Encounter::csvString() const {
  return ""; // TODO fill this out properly
}

bool
Encounter::isValid() const {
  return valid;
}

const Proximity&
Encounter::proximity() const
{
  return prox;
}

const PayloadData&
Encounter::payload() const
{
  return payloadData;
}

const Date&
Encounter::timestamp() const
{
  return date;
}

} // end namespace
} // end namespace
