//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BEACON_PAYLOAD_DATA_SUPPLIER_H
#define BEACON_PAYLOAD_DATA_SUPPLIER_H

#include "../extended/extended_data.h"
#include "../payload_data_supplier.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>

namespace herald {
namespace payload {
namespace beacon {

using namespace herald::payload::extended;
using namespace herald::datatype;

class BeaconPayloadDataSupplier : public PayloadDataSupplier {
public:
  BeaconPayloadDataSupplier() = default;
  virtual ~BeaconPayloadDataSupplier() = default;
};

class ConcreteBeaconPayloadDataSupplierV1 : public BeaconPayloadDataSupplier {
public:
  ConcreteBeaconPayloadDataSupplierV1(uint16_t countryCode, uint16_t stateCode, 
    uint32_t code, std::optional<ConcreteExtendedDataV1> extendedData);
  ~ConcreteBeaconPayloadDataSupplierV1() = default;

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
