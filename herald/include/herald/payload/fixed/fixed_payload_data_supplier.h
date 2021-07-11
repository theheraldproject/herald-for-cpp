//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_FIXED_PAYLOAD_DATA_SUPPLIER_H
#define HERALD_FIXED_PAYLOAD_DATA_SUPPLIER_H

#include "../payload_data_supplier.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace fixed {

using namespace herald::datatype;

using MYUINT32 = unsigned long;

class FixedPayloadDataSupplier {
public:
  FixedPayloadDataSupplier() = default;
  virtual ~FixedPayloadDataSupplier() = default;
};

class ConcreteFixedPayloadDataSupplierV1 : public FixedPayloadDataSupplier {
public:
  ConcreteFixedPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    std::uint64_t clientId);
  ~ConcreteFixedPayloadDataSupplierV1();

  PayloadData legacyPayload(const PayloadTimestamp timestamp, const Device& device);
  PayloadData payload(const PayloadTimestamp timestamp, const Device& device);
  PayloadData payload(const PayloadTimestamp timestamp);
  std::vector<PayloadData> payload(const Data& data);

private:
  uint16_t country;
  uint16_t state;
  uint64_t clientIdentifier;

  PayloadData mPayload;
};

}
}
}

#endif
