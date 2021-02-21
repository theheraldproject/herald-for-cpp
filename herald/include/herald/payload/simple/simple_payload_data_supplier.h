//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIMPLE_PAYLOAD_DATA_SUPPLIER_H
#define SIMPLE_PAYLOAD_DATA_SUPPLIER_H

#include "secret_key.h"
#include "../payload_data_supplier.h"
#include "../extended/extended_data.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;
using namespace herald::payload::extended;

using MYUINT32 = unsigned long;

class SimplePayloadDataSupplier : public PayloadDataSupplier {
public:
  SimplePayloadDataSupplier() = default;
  virtual ~SimplePayloadDataSupplier() = default;
};

class ConcreteSimplePayloadDataSupplierV1 : public SimplePayloadDataSupplier {
public:
  ConcreteSimplePayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk);
  ConcreteSimplePayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, ConcreteExtendedDataV1 ext);
  ConcreteSimplePayloadDataSupplierV1(const ConcreteSimplePayloadDataSupplierV1& from) = delete; // copy ctor deletion
  ConcreteSimplePayloadDataSupplierV1(ConcreteSimplePayloadDataSupplierV1&& from) = delete; // move ctor deletion
  ~ConcreteSimplePayloadDataSupplierV1();

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
