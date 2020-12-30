//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/beacon/beacon_payload_data_supplier.h"
#include "herald/datatype/data.h"

#include <optional>

namespace herald {
namespace payload {
namespace beacon {

class ConcreteBeaconPayloadDataSupplierV1::Impl {
public:
  Impl(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code, ConcreteExtendedDataV1 extendedData);
  Impl(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code);
  ~Impl();

  uint16_t country;
  uint16_t state;
  MYUINT32 code;
  std::optional<ConcreteExtendedDataV1> extendedData;

  PayloadData payload;
};

ConcreteBeaconPayloadDataSupplierV1::Impl::Impl(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code, ConcreteExtendedDataV1 ed)
  : country(countryCode), state(stateCode), code(code), extendedData(ed), payload()
{
  payload.append(uint8_t(0x30)); // Venue Beacon payload V1
  payload.append(countryCode);
  payload.append(stateCode);
  payload.append(std::uint32_t(code));
  auto edpl = ed.payload();
  if (edpl) {
    payload.append(*edpl);
  }
}

ConcreteBeaconPayloadDataSupplierV1::Impl::Impl(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code)
  : country(countryCode), state(stateCode), code(code), extendedData(), payload()
{
  payload.append(uint8_t(0x30)); // Venue Beacon payload V1
  payload.append(countryCode);
  payload.append(stateCode);
  payload.append(std::uint32_t(code));
}

ConcreteBeaconPayloadDataSupplierV1::Impl::~Impl()
{
  ;
}






ConcreteBeaconPayloadDataSupplierV1::ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    MYUINT32 code, ConcreteExtendedDataV1 extendedData)
  : BeaconPayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(countryCode,stateCode,code,extendedData))
{
  ;
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
    mImpl(std::make_unique<Impl>(countryCode,stateCode,code))
{
  ;
}

ConcreteBeaconPayloadDataSupplierV1::~ConcreteBeaconPayloadDataSupplierV1()
{
  ;
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return std::optional<PayloadData>();
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return mImpl->payload;
}

std::vector<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::payload(const Data& data)
{
  return std::vector<PayloadData>();
}

}
}
}
