//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "payload/beacon/beacon_payload_data_supplier.h"
#include "datatype/data.h"

#include <optional>

namespace herald {
namespace payload {
namespace beacon {

class ConcreteBeaconPayloadDataSupplierV1::Impl {
public:
  Impl(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, const std::optional<ConcreteExtendedDataV1>& extendedData);
  ~Impl();

  uint16_t country;
  uint16_t state;
  uint32_t code;
  std::optional<ConcreteExtendedDataV1> extendedData;

  PayloadData payload;
};

ConcreteBeaconPayloadDataSupplierV1::Impl::Impl(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, const std::optional<ConcreteExtendedDataV1>& ed)
  : country(countryCode), state(stateCode), code(code), extendedData(ed), payload()
{
  ;
  // TODO append to payload immediately and cache
}

ConcreteBeaconPayloadDataSupplierV1::Impl::~Impl()
{
  ;
}






ConcreteBeaconPayloadDataSupplierV1::ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, std::optional<ConcreteExtendedDataV1> extendedData)
  : mImpl(std::make_unique<Impl>(countryCode,stateCode,code,extendedData))
{
  ;
}

ConcreteBeaconPayloadDataSupplierV1::~ConcreteBeaconPayloadDataSupplierV1()
{
  ;
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestmap, const std::shared_ptr<Device> device)
{
  return std::optional<PayloadData>();
}

std::optional<PayloadData>
ConcreteBeaconPayloadDataSupplierV1::payload(const PayloadTimestamp timestmap, const std::shared_ptr<Device> device)
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
