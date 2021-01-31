//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/data/payload_data_formatter.h"
#include "herald/datatype/payload_data.h"

namespace herald::data {

using namespace herald::datatype;

std::string
ConcretePayloadDataFormatter::shortFormat(const PayloadData& payloadData) const noexcept
{
  return payloadData.shortName();
}

}