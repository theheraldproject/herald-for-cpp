//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PLACENAME_LOCATION_REFERENCE_H
#define PLACENAME_LOCATION_REFERENCE_H

#include "location_reference.h"

namespace herald {
namespace datatype {

struct PlacenameLocationReference : public LocationReference {
  PlacenameLocationReference(std::string n) : LocationReference(), name(n) { };
  ~PlacenameLocationReference() = default;
  
  std::string name;

  std::string description() {
    return "PLACE(name=" + name + ")";
  }
};


} // end namespace
} // end namespace

#endif