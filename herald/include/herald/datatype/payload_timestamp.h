//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PAYLOAD_TIMESTAMP_H
#define PAYLOAD_TIMESTAMP_H

#include "date.h"

namespace herald {
namespace datatype {

struct PayloadTimestamp {
  Date value;
};

} // end namespace
} // end namespace

#endif