//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIMPLE_K_H
#define SIMPLE_K_H

#include "secret_key.h"
#include "matching_key.h"
#include "contact_key.h"
#include "contact_identifier.h"
#include "../../datatype/data.h"
#include "../../datatype/time_interval.h"

#include <vector>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;

class K {
public:
  K() noexcept;
  K(int keyLength, int daysFor, int periodsInDay) noexcept;
  K(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning) noexcept;
  ~K() noexcept;

  static TimeInterval getEpoch() noexcept;

  int day(Date on) noexcept;

  int period(Date at) noexcept;

  const std::vector<MatchingKey>& matchingKeys(const SecretKey& secretKey) noexcept;

  const std::vector<ContactKey> contactKeys(const MatchingKey& matchingKey) noexcept;

  const ContactIdentifier contactIdentifier(const ContactKey& contactKey) noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

}
}
}

#endif
