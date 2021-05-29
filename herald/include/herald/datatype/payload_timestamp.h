//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_PAYLOAD_TIMESTAMP_H
#define HERALD_PAYLOAD_TIMESTAMP_H

#include "date.h"

namespace herald {
namespace datatype {

struct PayloadTimestamp {
  Date value;
};

} // end namespace
} // end namespace

#endif