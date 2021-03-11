//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/simple/k.h"
#include "herald/payload/simple/f.h"
#include "herald/payload/simple/secret_key.h"
#include "herald/payload/simple/matching_key.h"
#include "herald/payload/simple/matching_key_seed.h"
#include "herald/payload/simple/contact_key.h"
#include "herald/payload/simple/contact_key_seed.h"
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

  const int keyLength;
  const int daysFor;
  const int periodsInDay;
  const TimeInterval epoch;

  // instance data members (lazy populated)
  // std::vector<MatchingKey> matchingKeySet;
  // std::size_t lastSecretKeyHash;
};

K::Impl::Impl(int keyLength, int daysFor, int periodsInDay)
  : keyLength(keyLength), daysFor(daysFor), periodsInDay(periodsInDay), epoch(K::getEpoch())
{
  ;
}

K::Impl::Impl(int keyLength, int daysFor, int periodsInDay, TimeInterval epochBeginning)
  : keyLength(keyLength), daysFor(daysFor), periodsInDay(periodsInDay), epoch(epochBeginning)
{
  ;
}


K::Impl::~Impl() = default;


K::K() noexcept
  : mImpl(std::make_unique<Impl>(2048,2000,240))
{
  ;
}

K::K(const K& other) noexcept
  : mImpl(std::make_unique<Impl>(other.mImpl->keyLength, other.mImpl->daysFor,other.mImpl->periodsInDay, other.mImpl->epoch))
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
  return TimeInterval(0); // Jan 1st 1970, seconds
}

int
K::day(Date on) const noexcept {
  return (long)(on - mImpl->epoch) / 86400;
}

int
K::period(Date at) const noexcept {
  long seconds = (long)(at - mImpl->epoch) % 86400;
  return (seconds * mImpl->periodsInDay) / 86400; // more accurate
}

// const std::vector<MatchingKey>&
// K::matchingKeys(const SecretKey& secretKey) noexcept {
//   if (0 == mImpl->matchingKeySet.size()) {
//     // lazy initialisation
//     std::vector<MatchingKeySeed> matchingKeySeed(mImpl->daysFor + 1);
//     matchingKeySeed.reserve(mImpl->daysFor + 1);
//     matchingKeySeed[mImpl->daysFor] = MatchingKeySeed(F::h(secretKey));
//     for (int i = mImpl->daysFor - 1;i >=0; i--) {
//       matchingKeySeed[i].append(F::h(F::t(matchingKeySeed[i + 1])));
//     }

//     mImpl->matchingKeySet.reserve(mImpl->daysFor + 1);
//     for (int i = 0;i <= mImpl->daysFor;i++) {
//       mImpl->matchingKeySet.emplace_back();
//     }
    
//     // matching key on day 0 is derived from matching key seed on day 0 and day -1
//     MatchingKeySeed minusOne(F::h(F::t(matchingKeySeed[0])));
//     mImpl->matchingKeySet[0].append(F::h(F::xorData(matchingKeySeed[0],minusOne)));
    
//     // Matching key for day i is the hash of the matching key seed for day i xor i-1
//     for (int i = 1; i <= mImpl->daysFor;i++) {
//       mImpl->matchingKeySet[i].append(F::h(F::xorData(matchingKeySeed[i], matchingKeySeed[i - 1])));
//     }
//     // TODO set sk hash in this class to cache result
//   } else {
//     // TODO verify that the hash of the sk is the same as before
//   }
//   return mImpl->matchingKeySet;
// }

/// Low memory version of the key generator - generates keys between specified dates (inclusive)
/// Generates for just the first day now (for verification this fixes the memory issue)
/// TODO In future, it will generate them for between specified dates.
const MatchingKey
K::matchingKey(const SecretKey& secretKey, const int dayIdxFor) noexcept {
  // lazy initialisation
  MatchingKeySeed last;	
  MatchingKeySeed newSeed(F::h(secretKey));
  for (int i = mImpl->daysFor - 1;i >=dayIdxFor; i--) {
    newSeed.append(F::h(F::t(last)));
    last.clear();
    last.append(newSeed);
    newSeed.clear();
  }
  // At this point newSeed is empty and last holds the seed for day dayIdxFor
  
  // matching key on day 0 is derived from matching key seed on day dayIdxFor and day dayIdxFor-1
  MatchingKeySeed minusOne(F::h(F::t(last)));

  return MatchingKey(F::h(F::xorData(last,minusOne)));
}

// const std::vector<ContactKey>
// K::contactKeys(const MatchingKey& matchingKey) noexcept {
//   const int n = mImpl->periodsInDay;

//   std::vector<ContactKeySeed> contactKeySeed;
//   contactKeySeed.reserve(n + 1);

//   for (int i = 0;i <= n;i++) {
//     contactKeySeed.emplace_back();
//   }

//   contactKeySeed[n].append(F::h(matchingKey));
//   for (int j = n - 1;j >= 0;j--) {
//     contactKeySeed[j].append(F::h(F::t(contactKeySeed[j + 1])));
//   }

//   std::vector<ContactKey> contactKey;
//   contactKey.reserve(n + 1);

//   for (int i = 0;i <= n;i++) {
//     contactKey.emplace_back();
//   }
  
//   for (int j = 1;j <= n;j++) {
//     contactKey[j].append(F::h(F::xorData(contactKeySeed[j],contactKeySeed[j - 1])));
//   }

//   // Day 0 key now
//   ContactKeySeed minusOne(F::h(F::t(contactKeySeed[0])));
//   contactKey[0].append(F::h(F::xorData(contactKeySeed[0], minusOne)));

//   return contactKey;
// }

const ContactKey
K::contactKey(const SecretKey& secretKey, const int dayFor, const int periodFor) noexcept {
  const int n = mImpl->periodsInDay;

  auto mk = matchingKey(secretKey,dayFor);

  ContactKeySeed last(F::h(mk));
  ContactKeySeed lastMinusOne;
  for (int j = n - 1;j >= periodFor;j--) {
    lastMinusOne.clear();
    lastMinusOne.append(F::h(F::t(last)));
    last.clear();
    last.append(lastMinusOne);
  }
  // we now have lastMinusOne = contactKeySeed at periodFor

  // Day 0 key now
  ContactKeySeed minusOne(F::h(F::t(last)));

  return ContactKey(F::h(F::xorData(last, minusOne)));
}

// const ContactIdentifier
// K::contactIdentifier(const ContactKey& contactKey) noexcept {
//   return ContactIdentifier(F::t(contactKey, 16));
// }

const ContactIdentifier
K::contactIdentifier(const SecretKey& secretKey, const int dayFor,const int periodFor) noexcept {
  auto ck = contactKey(secretKey,dayFor,periodFor);
  return ContactIdentifier(F::t(ck, 16));
}


}
}
}
