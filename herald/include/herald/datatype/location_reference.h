//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef LOCATION_REFERENCE_H
#define LOCATION_REFERENCE_H

#include <string>

namespace herald {
namespace datatype {

struct LocationReference {
  virtual std::string description() = 0;
};


} // end namespace
} // end namespace

#endif