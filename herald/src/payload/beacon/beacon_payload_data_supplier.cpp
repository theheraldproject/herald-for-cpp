//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/beacon/beacon_payload_data_supplier.h"
#include "herald/datatype/data.h"

#include <optional>

namespace herald {
namespace payload {
namespace beacon {

ConcreteBeaconPayloadDataSupplierV1::ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code, ConcreteExtendedDataV1 extendedData)
  : BeaconPayloadDataSupplier(),
    country(countryCode), state(stateCode), code(code), extendedData(extendedData), mPayload()
{
  mPayload.append(uint8_t(0x30)); // Venue Beacon payload V1
  mPayload.append(countryCode);
  mPayload.append(stateCode);
  mPayload.append(std::uint32_t(code));
  auto edpl = extendedData.payload();
  // if (edpl.size() > 0) {
    mPayload.append(edpl);
  // }
}
// ConcreteBeaconPayloadDataSupplierV1::ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
//     unsigned int code, ConcreteExtendedDataV1 extendedData)
//   : BeaconPayloadDataSupplier(),
//     mImpl(std::make_unique<Impl>(countryCode,stateCode,(uint32_t)code,extendedData))
// {
//   ;
// }
ConcreteBeaconPayloadDataSupplierV1::ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code)
  : BeaconPayloadDataSupplier(),
    country(countryCode), state(stateCode), code(code), extendedData(), mPayload()
{
  mPayload.append(uint8_t(0x30)); // Venue Beacon payload V1
  mPayload.append(countryCode);
  mPayload.append(stateCode);
  mPayload.append(std::uint32_t(code));
}

ConcreteBeaconPayloadDataSupplierV1::~ConcreteBeaconPayloadDataSupplierV1()
{
  ;
}

PayloadData
ConcreteBeaconPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const Device& device)
{
  return PayloadData();
}

PayloadData
ConcreteBeaconPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const Device& device)
{
  return mPayload;
}

PayloadData
ConcreteBeaconPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp)
{
  return mPayload;
}

// std::vector<PayloadData>
// ConcreteBeaconPayloadDataSupplierV1::payload(const Data& data)
// {
//   return std::vector<PayloadData>();
// }

}
}
}
