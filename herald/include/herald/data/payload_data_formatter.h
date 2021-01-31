//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PAYLOAD_DATA_FORMATTER_H
#define PAYLOAD_DATA_FORMATTER_H

#include "../datatype/payload_data.h"

#include <string>

namespace herald::data {

using namespace herald::datatype;

class PayloadDataFormatter {
public:
  PayloadDataFormatter() = default;
  virtual ~PayloadDataFormatter() = default;

  virtual std::string shortFormat(const PayloadData& payloadData) const noexcept = 0;
};

class ConcretePayloadDataFormatter : public PayloadDataFormatter {
public:
  ConcretePayloadDataFormatter() = default;
  ~ConcretePayloadDataFormatter() = default;

  std::string shortFormat(const PayloadData& payloadData) const noexcept override;
};

}

#endif