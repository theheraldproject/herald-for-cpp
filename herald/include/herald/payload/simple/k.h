//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SIMPLE_K_H
#define HERALD_SIMPLE_K_H

#include "secret_key.h"
#include "matching_key.h"
#include "contact_key.h"
#include "contact_identifier.h"
#include "../../datatype/data.h"
#include "../../datatype/time_interval.h"

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;

class K {
public:
  K() noexcept;
  K(const K& other) noexcept;
  K(K&&) = delete;
  K(int keyLength, int daysFor, int periodsInDay) noexcept;
  K(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning) noexcept;
  ~K() noexcept;

  static TimeInterval getEpoch() noexcept;

  int day(Date on) const noexcept;

  int period(Date at) const noexcept;


  MatchingKey matchingKey(const SecretKey& secretKey, const int dayFor) noexcept;

  ContactKey contactKey(const SecretKey& secretKey, const int dayFor, const int periodFor) noexcept;

  ContactIdentifier contactIdentifier(const SecretKey& secretKey, const int dayFor, const int periodFor) noexcept;

  // NOTE I'm keeping the old functions here in case we need to use a caching version on another platform

  // const std::vector<MatchingKey>& matchingKeys(const SecretKey& secretKey) noexcept;

  // const std::vector<ContactKey> contactKeys(const MatchingKey& matchingKey) noexcept;

  // const ContactIdentifier contactIdentifier(const ContactKey& contactKey) noexcept;
  
private:
  const int keyLength;
  const int daysFor;
  const int periodsInDay;
  const TimeInterval epoch;
};

}
}
}

#endif
