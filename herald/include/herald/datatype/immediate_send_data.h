//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef IMMEDIATE_SEND_DATA_H
#define IMMEDIATE_SEND_DATA_H

#include "data.h"

namespace herald {
namespace datatype {

class ImmediateSendData : public Data {
public:
  ImmediateSendData(const Data& from);
  ~ImmediateSendData() = default;
};

} // end namespace
} // end namespace

#endif