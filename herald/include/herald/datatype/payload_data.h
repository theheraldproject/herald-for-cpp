//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_PAYLOAD_DATA_H
#define HERALD_PAYLOAD_DATA_H

#include "data.h"

namespace herald {
namespace datatype {

class PayloadData : public Data {
public:
  PayloadData();
  PayloadData(const Data& from);
  PayloadData(const std::byte* data, std::size_t length);
  PayloadData(std::byte repeating, std::size_t count);
  ~PayloadData() = default;

  std::string shortName() const;
  std::string toString() const;
};

} // end namespace
} // end namespace

#endif