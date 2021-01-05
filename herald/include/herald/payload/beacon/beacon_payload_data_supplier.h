//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BEACON_PAYLOAD_DATA_SUPPLIER_H
#define BEACON_PAYLOAD_DATA_SUPPLIER_H

#include "../extended/extended_data.h"
#include "../payload_data_supplier.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace beacon {

using namespace herald::payload::extended;
using namespace herald::datatype;

using MYUINT32 = unsigned long;

class BeaconPayloadDataSupplier : public PayloadDataSupplier {
public:
  BeaconPayloadDataSupplier() = default;
  virtual ~BeaconPayloadDataSupplier() = default;
};

class ConcreteBeaconPayloadDataSupplierV1 : public BeaconPayloadDataSupplier {
public:
  ConcreteBeaconPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    MYUINT32 code, ConcreteExtendedDataV1 extendedData);
  // ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
  //   unsigned int code, ConcreteExtendedDataV1 extendedData);
  ConcreteBeaconPayloadDataSupplierV1(std::uint16_t countryCode, std::uint16_t stateCode, 
    MYUINT32 code);
  ~ConcreteBeaconPayloadDataSupplierV1();

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
