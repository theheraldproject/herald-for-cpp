//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "payload/fixed/fixed_payload_data_supplier.h"
#include "datatype/data.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace fixed {

class ConcreteFixedPayloadDataSupplierV1::Impl {
public:
  Impl(std::uint16_t countryCode, std::uint16_t stateCode, uint64_t clientId);
  ~Impl();

  uint16_t country;
  uint16_t state;
  uint64_t clientIdentifier;

  PayloadData payload;
};


ConcreteFixedPayloadDataSupplierV1::Impl::Impl(std::uint16_t countryCode, std::uint16_t stateCode, 
    std::uint64_t clientId)
  : country(countryCode), state(stateCode), clientIdentifier(clientId), payload()
{
  payload.append(std::uint8_t(0x00)); // Fixed testing payload V1 (custom rangx 0x00-0x0f)
  payload.append(countryCode);
  payload.append(stateCode);
  payload.append(clientId);
}

ConcreteFixedPayloadDataSupplierV1::Impl::~Impl()
{
  ;
}






ConcreteFixedPayloadDataSupplierV1::ConcreteFixedPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    std::uint64_t clientId)
  : FixedPayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(countryCode,stateCode,clientId))
{
  ;
}

ConcreteFixedPayloadDataSupplierV1::~ConcreteFixedPayloadDataSupplierV1()
{
  ;
}

std::optional<PayloadData>
ConcreteFixedPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return std::optional<PayloadData>();
}

std::optional<PayloadData>
ConcreteFixedPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return std::optional<PayloadData>(mImpl->payload);
}

std::vector<PayloadData>
ConcreteFixedPayloadDataSupplierV1::payload(const Data& data)
{
  return std::vector<PayloadData>();
}

}
}
}
