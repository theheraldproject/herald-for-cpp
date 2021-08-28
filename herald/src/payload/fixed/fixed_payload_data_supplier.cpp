//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/fixed/fixed_payload_data_supplier.h"
#include "herald/datatype/data.h"

#include <optional>

namespace herald {
namespace payload {
namespace fixed {

ConcreteFixedPayloadDataSupplierV1::ConcreteFixedPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    std::uint64_t clientId)
  : FixedPayloadDataSupplier(),
    country(countryCode), state(stateCode), clientIdentifier(clientId), mPayload()
{
  mPayload.append(std::uint8_t(0x08)); // Fixed testing payload V1 (custom range with country/state codes are in 0x08-0x0f)
  mPayload.append(countryCode);
  mPayload.append(stateCode);
  mPayload.append(clientId);
  ;
}

ConcreteFixedPayloadDataSupplierV1::~ConcreteFixedPayloadDataSupplierV1() = default;

PayloadData
ConcreteFixedPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const Device& device)
{
  return PayloadData();
}

PayloadData
ConcreteFixedPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const Device& device)
{
  return mPayload;
}

PayloadData
ConcreteFixedPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp)
{
  return mPayload;
}

PayloadData
ConcreteFixedPayloadDataSupplierV1::payload(const Data& data)
{
  return PayloadData();
}

}
}
}
