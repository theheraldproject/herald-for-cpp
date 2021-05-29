//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PAYLOAD_SHARING_DATA_H
#define PAYLOAD_SHARING_DATA_H

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