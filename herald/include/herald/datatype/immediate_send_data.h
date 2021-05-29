//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_IMMEDIATE_SEND_DATA_H
#define HERALD_IMMEDIATE_SEND_DATA_H

#include "data.h"

namespace herald {
namespace datatype {

class ImmediateSendData : public Data {
public:
  ImmediateSendData();
  ImmediateSendData(const Data& from);
  ~ImmediateSendData() = default;
};

} // end namespace
} // end namespace

#endif