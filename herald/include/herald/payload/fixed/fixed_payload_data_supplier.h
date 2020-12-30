//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef FIXED_PAYLOAD_DATA_SUPPLIER_H
#define FIXED_PAYLOAD_DATA_SUPPLIER_H

#include "../payload_data_supplier.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace fixed {

using namespace herald::datatype;

using MYUINT32 = unsigned long;

class FixedPayloadDataSupplier : public PayloadDataSupplier {
public:
  FixedPayloadDataSupplier() = default;
  virtual ~FixedPayloadDataSupplier() = default;
};

class ConcreteFixedPayloadDataSupplierV1 : public FixedPayloadDataSupplier {
public:
  ConcreteFixedPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    std::uint64_t clientId);
  ~ConcreteFixedPayloadDataSupplierV1();

  std::optional<PayloadData> legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device) override;
  std::optional<PayloadData> payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device) override;
  std::vector<PayloadData> payload(const Data& data) override;

private:
  class Impl; // fwd decl
  std::unique_ptr<Impl> mImpl; // PIMPL idiom
};

}
}
}

#endif
