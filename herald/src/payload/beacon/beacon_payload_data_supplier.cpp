//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "payload/beacon/beacon_payload_data_supplier.h"
#include "datatype/data.h"

namespace herald {
namespace payload {
namespace beacon {

class ConcreteBeaconPayloadDataSupplierV1::Impl {
public:
  Impl(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, std::optional<ConcreteExtendedDataV1> extendedData);
  ~Impl();

  uint16_t country;
  uint16_t state;
  uint32_t code;
  std::optional<ConcreteExtendedDataV1> extendedData;

  PayloadData payload;
};

ConcreteBeaconPayloadDataSupplierV1::Impl::Impl(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, std::optional<ConcreteExtendedDataV1> extendedData)
  : country(countryCode), state(stateCode), code(code), extendedData(extendedData), payload()
{
  ;
  // TODO append to payload immediately and cache
}

ConcreteBeaconPayloadDataSupplierV1::Impl::~Impl()
{
  ;
}




ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, std::optional<ConcreteExtendedDataV1> extendedData)
  : mImpl(std::make_unique<Impl>(countryCode,stateCode,code,extendedData))
{
  ;
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestmap, const std::shared_ptr<Device> device)
{
  return nullptr_t;
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::payload(const PayloadTimestamp timestmap, const std::shared_ptr<Device> device)
{
  return mImpl->payload;
}

std::vector<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::payload(const Data& data)
{
  return mImpl->payload;
}

}
}
}
