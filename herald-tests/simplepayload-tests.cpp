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
    herald::payload::simple::SecretKey ks1(std::byte(0),2048);
    herald::payload::simple::SecretKey ks2(std::byte(0),2048);
    // Generate a third that is different
    herald::payload::simple::SecretKey ks3(std::byte(1),2048);

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

    bool km12eq = (km1 == km2);
    bool km13eq = (km1 == km3);
    bool km32eq = (km3 == km2);
    // same secret for matching
    REQUIRE(km12eq);
    // different keys yield different results
    REQUIRE(km13eq);
    REQUIRE(km32eq);
  }
}



TEST_CASE("payload-simple-basic", "[payload][simple][basic]") {
  SECTION("payload-simple-basic") {
    std::uint16_t country = 826;
    std::uint16_t state = 4;
    herald::payload::simple::SecretKey sk(std::byte(0x00),2048);
    herald::payload::simple::ConcreteSimplePayloadDataSupplierV1 pds(
      country,
      state,
      sk
    );
    auto pd = pds.payload(herald::datatype::PayloadTimestamp(),nullptr);

    REQUIRE(pd.has_value());
    REQUIRE(pd->size() == 21); // 1 version code, 2 country, 2 state, 16 clientId, no optional = 21
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
    REQUIRE(rcid1 == 0); // TODO verify
    REQUIRE(rcid2 == 0); // TODO verify
  }
}
