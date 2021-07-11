//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SIMPLE_PAYLOAD_DATA_SUPPLIER_H
#define HERALD_SIMPLE_PAYLOAD_DATA_SUPPLIER_H

#include "secret_key.h"
#include "k.h"
#include "../payload_data_supplier.h"
#include "../extended/extended_data.h"
#include "../../context.h"
#include "../../datatype/payload_timestamp.h"
#include "../../data/sensor_logger.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;
using namespace herald::payload::extended;

using MYUINT32 = unsigned long;

class SimplePayloadDataSupplier {
public:
  SimplePayloadDataSupplier() = default;
  virtual ~SimplePayloadDataSupplier() = default;
};

template <typename ContextT>
class ConcreteSimplePayloadDataSupplierV1 : public SimplePayloadDataSupplier {
public:
  ConcreteSimplePayloadDataSupplierV1(ContextT& context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k)
  : SimplePayloadDataSupplier(),
    ctx(context), country(countryCode), state(stateCode), secretKey(sk), k(k), 
    commonPayloadHeader(), extended(), day(-1), contactIdentifiers()
    HLOGGERINIT(ctx, "Sensor", "ConcreteSimplePayloadDataSupplierV1")
  {
    commonPayloadHeader.append(std::uint8_t(0x10)); // Simple payload V1
    commonPayloadHeader.append(country);
    commonPayloadHeader.append(state);

    HTDBG("About to call matching keys");
    // matchingKeys = k.matchingKeys(sk);
    HTDBG("Completed matching keys call");
  }
  ConcreteSimplePayloadDataSupplierV1(ContextT& context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k, ConcreteExtendedDataV1 ext)
  : SimplePayloadDataSupplier(),
    ctx(context), country(countryCode), state(stateCode), secretKey(sk), k(k), 
    commonPayloadHeader(), extended(ext), day(-1), contactIdentifiers()
    HLOGGERINIT(ctx, "Sensor", "ConcreteSimplePayloadDataSupplierV1")
  {
    commonPayloadHeader.append(std::uint8_t(0x10)); // Simple payload V1
    commonPayloadHeader.append(country);
    commonPayloadHeader.append(state);
  }
  ConcreteSimplePayloadDataSupplierV1(const ConcreteSimplePayloadDataSupplierV1& from) = delete; // copy ctor deletion
  ConcreteSimplePayloadDataSupplierV1(ConcreteSimplePayloadDataSupplierV1&& from) = delete; // move ctor deletion
  ~ConcreteSimplePayloadDataSupplierV1() = default;

  PayloadData legacyPayload(const PayloadTimestamp timestamp, const Device& device) {
    return PayloadData();
  }

  PayloadData payload(const PayloadTimestamp timestamp, const Device& device) {
    const int day = k.day(timestamp.value);
    const int period = k.period(timestamp.value);

    auto cid = k.contactIdentifier(secretKey,day,period);

    PayloadData p(commonPayloadHeader);
    // length
    if (extended.hasData()) {
      p.append(std::uint16_t(2 + extended.payload().value().size()));
    } else {
      p.append(std::uint16_t(2));
    }
    // contact id
    p.append(cid);
    // extended data
    if (extended.hasData()) {
      p.append(extended.payload().value());
    }

    return p;
  }

  PayloadData payload(const PayloadTimestamp timestamp) {
    const int day = k.day(timestamp.value);
    const int period = k.period(timestamp.value);

    auto cid = k.contactIdentifier(secretKey,day,period);

    PayloadData p(commonPayloadHeader);
    // length
    if (extended.hasData()) {
      p.append(std::uint16_t(2 + extended.payload().value().size()));
    } else {
      p.append(std::uint16_t(2));
    }
    // contact id
    p.append(cid);
    // extended data
    if (extended.hasData()) {
      p.append(extended.payload().value());
    }

    return p;
  }

  std::vector<PayloadData> payload(const Data& data) {
    return std::vector<PayloadData>();
  }

private:
  ContextT& ctx;
  const uint16_t country;
  const uint16_t state;
  const SecretKey secretKey;
  K k;

  PayloadData commonPayloadHeader;
  // std::vector<MatchingKey> matchingKeys;

  ConcreteExtendedDataV1 extended;

  // cached day/period info
  int day;
  std::vector<ContactIdentifier> contactIdentifiers;

  HLOGGER(ContextT);
};

}
}
}

#endif
