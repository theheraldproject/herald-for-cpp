//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("payload-simple-k-initialvalues", "[payload][simple][k][initialvalues]") {
  SECTION("payload-simple-k-initialvalues") {
    herald::payload::simple::K k; // default values

    REQUIRE(k.day(herald::datatype::Date(0)) == 0); // default on C++ to date of 'today', not a prior date, due to platform might not be able to support dates at all
  }
}

TEST_CASE("payload-simple-k-day", "[payload][simple][k][day]") {
  SECTION("payload-simple-k-day") {
    herald::payload::simple::K kEpoch(2048,2000,240,herald::datatype::TimeInterval(60*60*24)); // default values

    // Day before epoch - only works if epoch is not zero, so we hard set this k
    REQUIRE(kEpoch.day(herald::datatype::Date(60*60*24) - herald::datatype::TimeInterval(60*60*24)) == -1);
    
    
    herald::payload::simple::K k; // default values
    // Epoch day
    REQUIRE(k.day(herald::datatype::Date(0)) == 0);
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval::seconds(1)) == 0);
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval((60*60*24)-1)) == 0);
    // Day after epoch
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24)) == 1);
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24) + herald::datatype::TimeInterval::seconds(1)) == 1);
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24) + herald::datatype::TimeInterval((60*60*24)-1)) == 1);
    // 2 days after epoch
    REQUIRE(k.day(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24*2)) == 2);
  }
}

TEST_CASE("payload-simple-k-period", "[payload][simple][k][period]") {
  SECTION("payload-simple-k-period") {
    herald::payload::simple::K k(2048,2000,240,herald::datatype::TimeInterval(0)); // default values

    REQUIRE(k.period(herald::datatype::Date(0)) == 0);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval::seconds(1)) == 0);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval((60*5)+59)) == 0);
    // A period is 6 minutes long
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*6)) == 1);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*6) + herald::datatype::TimeInterval::seconds(1)) == 1);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*6*2) - herald::datatype::TimeInterval::seconds(1)) == 1);
    // last period of the day
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24) - herald::datatype::TimeInterval(60*6)) == 239);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24) - herald::datatype::TimeInterval(60*6) + herald::datatype::TimeInterval::seconds(1)) == 239);
    REQUIRE(k.period(herald::datatype::Date(0) + herald::datatype::TimeInterval(60*60*24) - herald::datatype::TimeInterval::seconds(1)) == 239);
  }
}

TEST_CASE("payload-simple-secretkey", "[payload][simple][secretkey]") {
  SECTION("payload-simple-secretkey") {
    // TODO use random source to generate secret key data
    REQUIRE(true == true);
  }
}



TEST_CASE("payload-simple-matchingkeys", "[payload][simple][matchingkeys]") {
  SECTION("payload-simple-matchingkeys") {
    // Generate two keys the same
    herald::payload::simple::SecretKey ks1;
    herald::payload::simple::SecretKey ks2;
    // Generate a third that is different
    herald::payload::simple::SecretKey ks3;
    int v = 0;
    for (int i = 0;i < 2048;i++) {
      ks1.append(std::byte(v));
      ks2.append(std::byte(v));
      ks3.append(std::byte(2048 - v));
      v++;
    }

    // Create basis for comparison
    herald::payload::simple::K k1;
    herald::payload::simple::K k2;
    herald::payload::simple::K k3;

    // Generate matching keys based on the same secret key
    const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);
    const std::vector<herald::payload::simple::MatchingKey>& km2 = k2.matchingKeys(ks2);
    const std::vector<herald::payload::simple::MatchingKey>& km3 = k3.matchingKeys(ks3);

    REQUIRE(km1.size() == 2001);
    REQUIRE(km2.size() == 2001);
    REQUIRE(km3.size() == 2001);

    // matching key is 32 bytes
    REQUIRE(km1.front().size() == 32);
    REQUIRE(km2.front().size() == 32);
    REQUIRE(km3.front().size() == 32);

    // ensure subsequent matching keys vary
    REQUIRE(km1[0] != km1[1]);
    REQUIRE(km2[0] != km2[1]);
    REQUIRE(km3[0] != km3[1]);

    REQUIRE(km1[2] != km1[100]);
    REQUIRE(km2[2] != km2[100]);
    REQUIRE(km3[2] != km3[100]);

    // ensure equal sequences' matching keys are equal
    REQUIRE(km1.front() == km2.front());
    REQUIRE(km3.front() != km2.front());

    bool km12eq = (km1 == km2);
    bool km13ne = (km1 != km3);
    bool km32ne = (km3 != km2);
    // same secret for matching
    REQUIRE(km12eq);
    // different keys yield different results
    REQUIRE(km13ne);
    REQUIRE(km32ne);
  }
}

TEST_CASE("payload-simple-contactkeys", "[payload][simple][contactkeys]") {
  SECTION("payload-simple-contactkeys") {
    herald::payload::simple::SecretKey ks1;
    int v = 0;
    for (int i = 0;i < 2048;i++) {
      ks1.append(std::byte(v));
      v++;
    }

    herald::payload::simple::K k1;
    const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);

    // generate contact keys based on the same matching key
    const std::vector<herald::payload::simple::ContactKey> kc1 = k1.contactKeys(km1[0]);
    const std::vector<herald::payload::simple::ContactKey> kc2 = k1.contactKeys(km1[0]);
    // Now generate a contact key based on a different matching key
    const std::vector<herald::payload::simple::ContactKey> kc3 = k1.contactKeys(km1[1]);

    // 241 contact keys per day (key 241 is not used)
    REQUIRE(kc1.size() == 241);
    REQUIRE(kc2.size() == 241);
    REQUIRE(kc3.size() == 241);

    // contact keys are 32 bytes
    REQUIRE(kc1.front().size() == 32);
    REQUIRE(kc2.front().size() == 32);
    REQUIRE(kc3.front().size() == 32);

    bool kc1eq2 = (kc1 == kc2);
    bool kc1ne3 = (kc1 != kc3);
    bool kc3ne2 = (kc3 != kc2);

    REQUIRE(kc1eq2);
    REQUIRE(kc1ne3);
    REQUIRE(kc3ne2);

    // All keys during the same day are different
    for (int i = 0;i < 239;i++) {
      for (int j = (i + 1); j < 240; j++) {
        REQUIRE(kc1[i] != kc1[j]);
        REQUIRE(kc2[i] != kc2[j]);
        REQUIRE(kc3[i] != kc3[j]);
      }
    }
  }
}

TEST_CASE("payload-simple-contactid", "[payload][simple][contactid]") {
  SECTION("payload-simple-contactid") {
    herald::payload::simple::SecretKey ks1;
    int v = 0;
    for (int i = 0;i < 2048;i++) {
      ks1.append(std::byte(v));
      v++;
    }

    herald::payload::simple::K k1;
    const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);
    const std::vector<herald::payload::simple::ContactKey> kc1 = k1.contactKeys(km1[0]);

    // generate contact identifiers
    herald::payload::simple::ContactIdentifier ic1 = k1.contactIdentifier(kc1[0]);
    herald::payload::simple::ContactIdentifier ic2 = k1.contactIdentifier(kc1[0]);
    herald::payload::simple::ContactIdentifier ic3 = k1.contactIdentifier(kc1[1]);

    REQUIRE(ic1.size() == 16);
    REQUIRE(ic2.size() == 16);
    REQUIRE(ic3.size() == 16);

    bool ic1eq2 = (ic1 == ic2);
    bool ic1ne3 = (ic1 != ic3);
    bool ic3ne2 = (ic3 != ic2);

    REQUIRE(ic1eq2);
    REQUIRE(ic1ne3);
    REQUIRE(ic3ne2);
  }
}


TEST_CASE("payload-simple-basic", "[payload][simple][basic]") {
  SECTION("payload-simple-basic") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::uint16_t country = 826;
    std::uint16_t state = 4;
    herald::payload::simple::K k;
    herald::payload::simple::SecretKey sk(std::byte(0x00),2048);
    herald::payload::simple::ConcreteSimplePayloadDataSupplierV1 pds(
      ctx,
      country,
      state,
      sk,
      k
    );
    auto pd = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)},nullptr);

    REQUIRE(pd.has_value());
    REQUIRE(pd->size() == 23); // 1 version code, 2 country, 2 state, 2 remainder length, 16 clientId, no optional = 23
    std::uint8_t rpidversion = 0;
    std::uint16_t rc = 0;
    std::uint16_t rs = 0;
    std::uint64_t rcid1 = 0;
    std::uint64_t rcid2 = 0;
    REQUIRE(pd->uint8(0,rpidversion));
    REQUIRE(pd->uint16(1,rc));
    REQUIRE(pd->uint16(3,rs));
    REQUIRE(pd->uint64(5,rcid1));
    REQUIRE(pd->uint64(13,rcid2));
    REQUIRE(rpidversion == std::uint8_t(0x10)); // https://vmware.github.io/herald/specs/payload-simple
    REQUIRE(rc == country);
    REQUIRE(rs == state);
    REQUIRE(rcid1 != 0); // TODO verify
    REQUIRE(rcid2 != 0); // TODO verify
  }
}

TEST_CASE("payload-simple-payloadbounds", "[payload][simple][payloadbounds]") {
  SECTION("payload-simple-payloadbounds") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::uint16_t country = 826;
    std::uint16_t state = 4;
    herald::payload::simple::K k;
    herald::payload::simple::SecretKey sk(std::byte(0x00),2048);
    herald::payload::simple::ConcreteSimplePayloadDataSupplierV1 pds(
      ctx,
      country,
      state,
      sk,
      k
    );

    // same payload in same period - basis is 0
    auto p1start = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)}, nullptr);
    auto p1samestart = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)}, nullptr);
    auto p1end = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date((6 * 60) - 1)}, nullptr);
    auto p2start = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(6 * 60)}, nullptr);
    REQUIRE(p1start.has_value());
    REQUIRE(p1samestart.has_value());
    REQUIRE(p1end.has_value());
    REQUIRE(p2start.has_value());
    REQUIRE(p1start == p1samestart);
    REQUIRE(p1start == p1end);
    REQUIRE(p1start != p2start);

  }
}