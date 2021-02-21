//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/simple/k.h"
#include "herald/payload/simple/secret_key.h"
#include "herald/payload/simple/matching_key.h"
#include "herald/payload/simple/contact_key.h"
#include "herald/payload/simple/contact_identifier.h"
#include "herald/datatype/data.h"
#include "herald/datatype/time_interval.h"

#include <vector>

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;


class K::Impl {
public:
  Impl(int keyLength, int daysFor, int periodsInDay);
  Impl(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning);
  ~Impl();

  int keyLength;
  int daysFor;
  int periodsInDay;
  TimeInterval epoch;

  // instance data members (lazy populated)
  std::vector<MatchingKey> matchingKeySet;
  std::vector<ContactKey> contactKeySet;
  std::size_t lastSecretKeyHash;
  std::size_t lastContactKeyHash;
};

K::Impl::Impl(int keyLength, int daysFor, int periodsInDay)
  : keyLength(keyLength), daysFor(daysFor), periodsInDay(periodsInDay), epoch(K::getEpoch()), matchingKeySet(), contactKeySet(), lastSecretKeyHash(0), lastContactKeyHash(0)
{
  ;
}

K::Impl::Impl(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning)
  : keyLength(keyLength), daysFor(daysFor), periodsInDay(periodsInDay), epoch(epochBeginning), matchingKeySet(), contactKeySet(), lastSecretKeyHash(0), lastContactKeyHash(0)
{
  ;
}


K::Impl::~Impl() = default;


K::K() noexcept
  : mImpl(std::make_unique<Impl>(2048,2000,240))
{
  ;
}

K::K(int keyLength, int daysFor, int periodsInDay) noexcept
  : mImpl(std::make_unique<Impl>(keyLength,daysFor,periodsInDay))
{
  ;
}

K::K(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning) noexcept
  : mImpl(std::make_unique<Impl>(keyLength,daysFor,periodsInDay,epochBeginning))
{
  ;
}

K::~K() noexcept = default;

TimeInterval
K::getEpoch() noexcept {
  // TODO real implementation
  return TimeInterval(0);
}

int
K::day(Date on) noexcept {
  return (long)(on - mImpl->epoch) / 86400;
}

int
K::period(Date at) noexcept {
  long seconds = (long)(at - mImpl->epoch) % 86400;
  return (seconds * mImpl->periodsInDay) / 86400; // more accurate
}

const std::vector<MatchingKey>&
K::matchingKeys(const SecretKey& secretKey) noexcept {
  // TODO real implementation
  return mImpl->matchingKeySet;
}

const std::vector<ContactKey>
K::contactKeys(const MatchingKey& matchingKey) noexcept {
  // TODO real implementation
  return mImpl->contactKeySet;
}

const ContactIdentifier
K::contactIdentifier(const ContactKey& contactKey) noexcept {
  // TODO real implementation
  return ContactIdentifier();
}


}
}
}
