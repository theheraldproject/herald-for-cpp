//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/simple/simple_payload_data_supplier.h"
#include "herald/payload/extended/extended_data.h"
#include "herald/datatype/data.h"

#include <optional>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::payload::extended;

class ConcreteSimplePayloadDataSupplierV1::Impl {
public:
  Impl(std::uint16_t countryCode, std::uint16_t stateCode, SecretKey sk);
  Impl(std::uint16_t countryCode, std::uint16_t stateCode, SecretKey sk, ConcreteExtendedDataV1 ext);
  ~Impl();

  uint16_t country;
  uint16_t state;
  SecretKey secretKey;

  PayloadData payload;

  ConcreteExtendedDataV1 extended;
};


ConcreteSimplePayloadDataSupplierV1::Impl::Impl(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk)
  : country(countryCode), state(stateCode), secretKey(sk), payload(), extended()
{
  payload.append(std::uint8_t(0x10)); // Simple payload V1
  payload.append(country);
  payload.append(state);

  // TODO generate data from secret key
  payload.append(std::uint64_t(0));
  payload.append(std::uint64_t(0));
}


ConcreteSimplePayloadDataSupplierV1::Impl::Impl(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, ConcreteExtendedDataV1 ext)
  : country(countryCode), state(stateCode), secretKey(sk), payload(), extended(ext)
{
  payload.append(std::uint8_t(0x10)); // Simple payload V1
  payload.append(country);
  payload.append(state);

  // TODO generate data from secret key
  payload.append(std::uint64_t(0));
  payload.append(std::uint64_t(0));

  auto extPayload = extended.payload();
  if (extPayload.has_value()) {
    payload.append(extPayload.value());
  }
}

ConcreteSimplePayloadDataSupplierV1::Impl::~Impl()
{
  ;
}






ConcreteSimplePayloadDataSupplierV1::ConcreteSimplePayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk)
  : SimplePayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(countryCode,stateCode,sk))
{
  ;
}

ConcreteSimplePayloadDataSupplierV1::ConcreteSimplePayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, ConcreteExtendedDataV1 ext)
  : SimplePayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(countryCode,stateCode,sk,ext))
{
  ;
}

ConcreteSimplePayloadDataSupplierV1::~ConcreteSimplePayloadDataSupplierV1()
{
  ;
}

std::optional<PayloadData>
ConcreteSimplePayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return std::optional<PayloadData>();
}

std::optional<PayloadData>
ConcreteSimplePayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  // TODO rotate this as required
  return std::optional<PayloadData>(mImpl->payload);
}

std::vector<PayloadData>
ConcreteSimplePayloadDataSupplierV1::payload(const Data& data)
{
  return std::vector<PayloadData>();
}

}
}
}
