//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BEACON_PAYLOAD_DATA_SUPPLIER_H
#define HERALD_BEACON_PAYLOAD_DATA_SUPPLIER_H

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

class BeaconPayloadDataSupplier {
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

  PayloadData legacyPayload(const PayloadTimestamp timestamp, const Device& device);
  PayloadData payload(const PayloadTimestamp timestamp, const Device& device);
  PayloadData payload(const PayloadTimestamp timestamp);
  std::vector<PayloadData> payload(const Data& data);

private:
  uint16_t country;
  uint16_t state;
  MYUINT32 code;
  ConcreteExtendedDataV1 extendedData;

  PayloadData mPayload;
};

}
}
}

#endif
