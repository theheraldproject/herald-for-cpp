//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/simple/simple_payload_data_supplier.h"
#include "herald/payload/simple/contact_identifier.h"
#include "herald/payload/simple/k.h"
#include "herald/payload/simple/matching_key.h"
#include "herald/payload/extended/extended_data.h"
#include "herald/datatype/data.h"
#include "herald/context.h"
#include "herald/data/sensor_logger.h"

#include <optional>
#include <memory>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::payload::extended;

class ConcreteSimplePayloadDataSupplierV1::Impl {
public:
  Impl(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, SecretKey sk, K k);
  Impl(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, SecretKey sk, K k, ConcreteExtendedDataV1 ext);
  ~Impl();

  std::shared_ptr<Context> ctx;
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

  HLOGGER
};


ConcreteSimplePayloadDataSupplierV1::Impl::Impl(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k)
  : ctx(context), country(countryCode), state(stateCode), secretKey(sk), k(k), commonPayloadHeader(), extended(), day(-1), contactIdentifiers()
  HLOGGERINIT(ctx, "Sensor", "ConcreteSimplePayloadDataSupplierV1")
{
  commonPayloadHeader.append(std::uint8_t(0x10)); // Simple payload V1
  commonPayloadHeader.append(country);
  commonPayloadHeader.append(state);

  HTDBG("About to call matching keys");
  // matchingKeys = k.matchingKeys(sk);
  HTDBG("Completed matching keys call");

  // // add length (no extended data)
  // payload.append(std::uint16_t(2));

  // // TODO generate data from secret key
  // payload.append(std::uint64_t(0));
  // payload.append(std::uint64_t(0));
}


ConcreteSimplePayloadDataSupplierV1::Impl::Impl(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k, ConcreteExtendedDataV1 ext)
  : ctx(context), country(countryCode), state(stateCode), secretKey(sk), k(k), commonPayloadHeader(), extended(ext), day(-1), contactIdentifiers()
  HLOGGERINIT(ctx, "Sensor", "ConcreteSimplePayloadDataSupplierV1")
{
  commonPayloadHeader.append(std::uint8_t(0x10)); // Simple payload V1
  commonPayloadHeader.append(country);
  commonPayloadHeader.append(state);

  // matchingKeys = k.matchingKeys(sk);

  // // add length (with extended data)
  // if (extended.hasData()) {
  //   payload.append(std::uint16_t(2 + extended.payload().value().size()));
  // } else {
  //   payload.append(std::uint16_t(2));
  // }

  // // TODO generate data from secret key
  // payload.append(std::uint64_t(0));
  // payload.append(std::uint64_t(0));

  // auto extPayload = extended.payload();
  // if (extPayload.has_value()) {
  //   payload.append(extPayload.value());
  // }
}

ConcreteSimplePayloadDataSupplierV1::Impl::~Impl()
{
  ;
}






ConcreteSimplePayloadDataSupplierV1::ConcreteSimplePayloadDataSupplierV1(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k)
  : SimplePayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(context, countryCode,stateCode,sk,k))
{
  ;
}

ConcreteSimplePayloadDataSupplierV1::ConcreteSimplePayloadDataSupplierV1(std::shared_ptr<Context> context, std::uint16_t countryCode, std::uint16_t stateCode, 
    SecretKey sk, K k, ConcreteExtendedDataV1 ext)
  : SimplePayloadDataSupplier(),
    mImpl(std::make_unique<Impl>(context, countryCode,stateCode,sk,k,ext))
{
  ;
}

ConcreteSimplePayloadDataSupplierV1::~ConcreteSimplePayloadDataSupplierV1()
{
  ;
}

std::optional<PayloadData>
ConcreteSimplePayloadDataSupplierV1::legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  return {};
}

std::optional<PayloadData>
ConcreteSimplePayloadDataSupplierV1::payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device)
{
  const int day = mImpl->k.day(timestamp.value);
  const int period = mImpl->k.period(timestamp.value);

  // if (!(day >= 0 && day < mImpl->matchingKeys.size())) {
  //   HERR("Contact identifier out of day range");
  //   return {};
  // }

  auto cid = mImpl->k.contactIdentifier(mImpl->secretKey,day,period);

  // if (-1 == mImpl->day || day != mImpl->day) {
  //   // generate new matching keys
  //   mImpl->day = day;
  //   auto contactKeys = mImpl->k.contactKeys(mImpl->matchingKeys[day]);
  //   mImpl->contactIdentifiers.clear();
  //   mImpl->contactIdentifiers.reserve(contactKeys.size());
  //   for (int i = 0;i < contactKeys.size();i++) {
  //     mImpl->contactIdentifiers.emplace_back();
  //   }
  //   for (int i = contactKeys.size() - 1;i >= 0;i--) {
  //     mImpl->contactIdentifiers[i].append(mImpl->k.contactIdentifier(contactKeys[i]));
  //   }
  // }

  // contact identifiers is always populated, so no error condition check here

  // if (!(period >=0 && period < mImpl->contactIdentifiers.size())) {
  //   HERR("Contact identifier out of period range");
  //   return {};
  // }

  // Defensive check
  // if (mImpl->contactIdentifiers[period].size() != 16) {
  //   HERR("Contact identifier not 16 bytes");
  //   return {};
  // }

  PayloadData p(mImpl->commonPayloadHeader);
  // length
  if (mImpl->extended.hasData()) {
    p.append(std::uint16_t(2 + mImpl->extended.payload().value().size()));
  } else {
    p.append(std::uint16_t(2));
  }
  // contact id
  p.append(cid);
  // extended data
  if (mImpl->extended.hasData()) {
    p.append(mImpl->extended.payload().value());
  }

  return std::optional<PayloadData>{p};
}

std::vector<PayloadData>
ConcreteSimplePayloadDataSupplierV1::payload(const Data& data)
{
  return std::vector<PayloadData>();
}

}
}
}
