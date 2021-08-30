//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_PAYLOAD_SHARING_DATA_H
#define HERALD_PAYLOAD_SHARING_DATA_H

#include "data.h"
#include "rssi.h"

namespace herald {
namespace datatype {

struct PayloadSharingData {
  RSSI rssi;
  Data data;
};

} // end namespace
} // end namespace

#endif