//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef LOCATION_H
#define LOCATION_H

#include "location_reference.h"
#include "date.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class Location {
public:
  Location(std::shared_ptr<LocationReference> value, Date start, Date end);
  ~Location();

  std::string description() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};

} // end namespace
} // end namespace

#endif