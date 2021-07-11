//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

// Test F first

TEST_CASE("payload-f-hash", "[payload][f][hash]") {
  SECTION("payload-f-hash") {
    herald::datatype::Data d;
    auto data = herald::payload::simple::F::h(d);

    REQUIRE(data.size() == 32);
  }
}

#ifdef __ZEPHYR__
TEST_CASE("payload-f-hash-known-result", "[payload][f][hash][known-result]") {
  SECTION("payload-f-hash-known-result") {
    herald::payload::simple::SecretKey sk(std::byte(0),2048); // known blank 2048 byte key
    REQUIRE(sk.size() == 2048);
    std::uint8_t value;
    REQUIRE(sk.uint8(0,value));
    REQUIRE(value == 0);
    REQUIRE(sk.uint8(2047,value));
    REQUIRE(value == 0);

    // std::string hex = sk.hexEncodedString();
    // REQUIRE("flibble" == hex);
    
    auto data = herald::payload::simple::F::h(sk);
    REQUIRE(data.size() == 32);

    std::string encoded = herald::datatype::Base64String::encode(data).encoded();
    // hex is: e5a00aa9991ac8a5ee3109844d84a55583bd20572ad3ffcd42792f3c36b183ad
    std::string expected("5aAKqZkayKXuMQmETYSlVYO9IFcq0//NQnkvPDaxg60=");

    herald::datatype::Base64String decoded;
    bool ok = herald::datatype::Base64String::from(expected,decoded);
    REQUIRE(ok);
    auto decodedData = decoded.decode();
    REQUIRE(decodedData.size() == 32);
    REQUIRE(decodedData == data);


    REQUIRE(expected == encoded);
  }
}
#endif

TEST_CASE("payload-f-trim-half", "[payload][f][trim-half]") {
  SECTION("payload-f-trim-half") {
    herald::datatype::Data dEmpty;
    auto dataEmptyHalfed = herald::payload::simple::F::t(dEmpty);
    REQUIRE(dataEmptyHalfed.size() == 0);
    
    herald::datatype::Data dOne(std::byte(1),1);
    auto dataOneHalfed = herald::payload::simple::F::t(dOne);
    REQUIRE(dataOneHalfed.size() == 0);
    
    herald::datatype::Data dTwo(std::byte(5),2);
    auto dataTwoHalfed = herald::payload::simple::F::t(dTwo);
    REQUIRE(dataTwoHalfed.size() == 1);
    
    herald::datatype::Data dEight(std::byte(7),8);
    auto dataEightHalfed = herald::payload::simple::F::t(dEight);
    REQUIRE(dataEightHalfed.size() == 4);
    
    herald::datatype::Data dThirtyTwo(std::byte(9),32);
    auto dataThirtyTwoHalfed = herald::payload::simple::F::t(dThirtyTwo);
    REQUIRE(dataThirtyTwoHalfed.size() == 16);
  }
}

TEST_CASE("payload-f-trim-arbitrary-empty", "[payload][f][arbitrary-empty]") {
  SECTION("payload-f-arbitrary-empty") {
    herald::datatype::Data d;
    auto data = herald::payload::simple::F::t(d,3);
    REQUIRE(data.size() == 0);
  }
}

TEST_CASE("payload-f-trim-arbitrary-one", "[payload][f][arbitrary-one]") {
  SECTION("payload-f-arbitrary-one") {
    herald::datatype::Data d(std::byte(1),1);
    auto data = herald::payload::simple::F::t(d,3);
    REQUIRE(data.size() == 1);
  }
}

TEST_CASE("payload-f-trim-arbitrary-two", "[payload][f][arbitrary-two]") {
  SECTION("payload-f-arbitrary-two") {
    herald::datatype::Data d(std::byte(1),2);
    auto data = herald::payload::simple::F::t(d,3);
    REQUIRE(data.size() == 2);
  }
}

TEST_CASE("payload-f-trim-arbitrary-three", "[payload][f][arbitrary-three]") {
  SECTION("payload-f-arbitrary-three") {
    herald::datatype::Data d(std::byte(1),3);
    auto data = herald::payload::simple::F::t(d,3);
    REQUIRE(data.size() == 3);
  }
}

TEST_CASE("payload-f-trim-arbitrary-four", "[payload][f][arbitrary-four]") {
  SECTION("payload-f-arbitrary-four") {
    herald::datatype::Data d(std::byte(1),4);
    auto data = herald::payload::simple::F::t(d,3);
    REQUIRE(data.size() == 3);
  }
}

TEST_CASE("payload-f-xor-zeros", "[payload][f][xor-zeros]") {
  SECTION("payload-f-xor-zeros") {
    herald::datatype::Data dZero(std::byte(0),1);
    auto data = herald::payload::simple::F::xorData(dZero,dZero);
    REQUIRE(data.size() == 1);
    REQUIRE(data == dZero);
  }
}

TEST_CASE("payload-f-xor-ones", "[payload][f][xor-ones]") {
  SECTION("payload-f-xor-ones") {
    herald::datatype::Data dZero(std::byte(0),1);
    herald::datatype::Data dOne(std::byte(0),1);
    auto data = herald::payload::simple::F::xorData(dOne,dOne);
    REQUIRE(data.size() == 1);
    REQUIRE(data == dZero);
  }
}

TEST_CASE("payload-f-xor-zero-one", "[payload][f][xor-zero-one]") {
  SECTION("payload-f-xor-zero-one") {
    herald::datatype::Data dZero(std::byte(0),1);
    herald::datatype::Data dOne(std::byte(0),1);
    auto data = herald::payload::simple::F::xorData(dZero,dOne);
    REQUIRE(data.size() == 1);
    REQUIRE(data == dOne);
  }
}

TEST_CASE("payload-f-xor-one-zero", "[payload][f][xor-one-zero]") {
  SECTION("payload-f-xor-one-zero") {
    herald::datatype::Data dZero(std::byte(0),1);
    herald::datatype::Data dOne(std::byte(0),1);
    auto data = herald::payload::simple::F::xorData(dOne,dZero);
    REQUIRE(data.size() == 1);
    REQUIRE(data == dOne);
  }
}

TEST_CASE("payload-f-xor-multiple", "[payload][f][xor-multiple]") {
  SECTION("payload-f-xor-multiple") {
    herald::datatype::Data dZero(std::byte(0),1);
    herald::datatype::Data dOne(std::byte(1),1);
    herald::datatype::Data dZeroZero(std::byte(0),2);
    herald::datatype::Data dZeroOne = dZero;
    dZeroOne.append(dOne);
    herald::datatype::Data dOneZero = dOne;
    dOneZero.append(dZero);
    herald::datatype::Data dOneOne(std::byte(1),2);

    std::uint8_t first;
    std::uint8_t second;
    INFO(dZeroOne);
    bool okFirst = dZeroOne.uint8(0,first);
    bool okSecond = dZeroOne.uint8(1,second);
    REQUIRE(okFirst);
    REQUIRE(okSecond);
    REQUIRE(0 == first);
    REQUIRE(1 == second);
    INFO(dOneZero);
    okFirst = dOneZero.uint8(0,first);
    okSecond = dOneZero.uint8(1,second);
    REQUIRE(okFirst);
    REQUIRE(okSecond);
    REQUIRE(1 == first);
    REQUIRE(0 == second);

    auto data = herald::payload::simple::F::xorData(dZeroZero,dZeroZero);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dZeroZero);

    data = herald::payload::simple::F::xorData(dZeroZero,dZeroOne);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dZeroOne);

    data = herald::payload::simple::F::xorData(dOneZero,dZeroZero);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dOneZero);

    data = herald::payload::simple::F::xorData(dOneOne,dOneOne);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dZeroZero);

    data = herald::payload::simple::F::xorData(dOneOne,dZeroOne);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dOneZero);

    data = herald::payload::simple::F::xorData(dOneZero,dOneOne);
    REQUIRE(data.size() == 2);
    REQUIRE(data == dZeroOne);
  }
}



// Now test K

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
    // const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);
    // const std::vector<herald::payload::simple::MatchingKey>& km2 = k2.matchingKeys(ks2);
    // const std::vector<herald::payload::simple::MatchingKey>& km3 = k3.matchingKeys(ks3);

    // REQUIRE(km1.size() == 2001);
    // REQUIRE(km2.size() == 2001);
    // REQUIRE(km3.size() == 2001);
    

    auto mk10 = k1.matchingKey(ks1,0);
    auto mk11 = k1.matchingKey(ks1,1);
    auto mk20 = k2.matchingKey(ks2,0);
    auto mk21 = k2.matchingKey(ks2,1);
    auto mk30 = k3.matchingKey(ks3,0);
    auto mk31 = k3.matchingKey(ks3,1);

    // matching key is 32 bytes
    REQUIRE(mk10.size() == 32);
    REQUIRE(mk20.size() == 32);
    REQUIRE(mk30.size() == 32);

    // ensure subsequent matching keys vary
    REQUIRE(mk10 != mk11);
    REQUIRE(mk20 != mk21);
    REQUIRE(mk30 != mk31);

    auto mk12 = k1.matchingKey(ks1,2);
    auto mk1100 = k1.matchingKey(ks1,100);
    auto mk22 = k2.matchingKey(ks2,2);
    auto mk2100 = k2.matchingKey(ks2,100);
    auto mk32 = k3.matchingKey(ks3,2);
    auto mk3100 = k3.matchingKey(ks3,100);
    REQUIRE(mk12 != mk1100);
    REQUIRE(mk22 != mk2100);
    REQUIRE(mk32 != mk3100);

    // ensure equal sequences' matching keys are equal
    REQUIRE(mk10 == mk20);
    REQUIRE(mk30 != mk20);

    // bool km12eq = (km1 == km2);
    // bool km13ne = (km1 != km3);
    // bool km32ne = (km3 != km2);
    // // same secret for matching
    // REQUIRE(km12eq);
    // // different keys yield different results
    // REQUIRE(km13ne);
    // REQUIRE(km32ne);
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
    // const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);

    // generate contact keys based on the same matching key
    // const std::vector<herald::payload::simple::ContactKey> kc1 = k1.contactKey(0);
    // const std::vector<herald::payload::simple::ContactKey> kc2 = k1.contactKey(0);
    // // Now generate a contact key based on a different matching key
    // const std::vector<herald::payload::simple::ContactKey> kc3 = k1.contactKey(1);

    // 241 contact keys per day (key 241 is not used)
    // REQUIRE(kc1.size() == 241);
    // REQUIRE(kc2.size() == 241);
    // REQUIRE(kc3.size() == 241);

    auto ck10 = k1.contactKey(ks1,0,0);
    auto ck11 = k1.contactKey(ks1,0,1);
    auto ck20 = k1.contactKey(ks1,0,0);
    auto ck21 = k1.contactKey(ks1,0,1);
    auto ck30 = k1.contactKey(ks1,1,0);
    auto ck31 = k1.contactKey(ks1,1,1);

    // contact keys are 32 bytes
    REQUIRE(ck10.size() == 32);
    REQUIRE(ck20.size() == 32);
    REQUIRE(ck30.size() == 32);

    // bool kc1eq2 = (kc1 == kc2);
    // bool kc1ne3 = (kc1 != kc3);
    // bool kc3ne2 = (kc3 != kc2);

    // REQUIRE(kc1eq2);
    // REQUIRE(kc1ne3);
    // REQUIRE(kc3ne2);

    // // All keys during the same day are different
    // for (int i = 0;i < 239;i++) {
    //   for (int j = (i + 1); j < 240; j++) {
    //     REQUIRE(kc1[i] != kc1[j]);
    //     REQUIRE(kc2[i] != kc2[j]);
    //     REQUIRE(kc3[i] != kc3[j]);
    //   }
    // }
    REQUIRE(ck10 != ck11);
    REQUIRE(ck20 != ck21);
    REQUIRE(ck30 != ck31);
    REQUIRE(ck10 == ck20);
    REQUIRE(ck30 != ck31);
    REQUIRE(ck31 != ck11);
    REQUIRE(ck21 == ck11);
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
    // const std::vector<herald::payload::simple::MatchingKey>& km1 = k1.matchingKeys(ks1);
    // const std::vector<herald::payload::simple::ContactKey> kc1 = k1.contactKeys(km1[0]);

    // generate contact identifiers
    // herald::payload::simple::ContactIdentifier ic1 = k1.contactIdentifier(kc1[0]);
    // herald::payload::simple::ContactIdentifier ic2 = k1.contactIdentifier(kc1[0]);
    // herald::payload::simple::ContactIdentifier ic3 = k1.contactIdentifier(kc1[1]);
    herald::payload::simple::ContactIdentifier ic1 = k1.contactIdentifier(ks1,0,0);
    herald::payload::simple::ContactIdentifier ic2 = k1.contactIdentifier(ks1,0,0);
    herald::payload::simple::ContactIdentifier ic3 = k1.contactIdentifier(ks1,1,1);

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
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
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
    BlankDevice bd;
    auto pd = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)},bd);

    REQUIRE(pd.size() == 23); // 1 version code, 2 country, 2 state, 2 remainder length, 16 clientId, no optional = 23
    std::uint8_t rpidversion = 0;
    std::uint16_t rc = 0;
    std::uint16_t rs = 0;
    std::uint64_t rcid1 = 0;
    std::uint64_t rcid2 = 0;
    REQUIRE(pd.uint8(0,rpidversion));
    REQUIRE(pd.uint16(1,rc));
    REQUIRE(pd.uint16(3,rs));
    REQUIRE(pd.uint64(5,rcid1));
    REQUIRE(pd.uint64(13,rcid2));
    REQUIRE(rpidversion == std::uint8_t(0x10)); // https://heraldprox.io/specs/payload-simple
    REQUIRE(rc == country);
    REQUIRE(rs == state);
    REQUIRE(rcid1 != 0); // TODO verify
    REQUIRE(rcid2 != 0); // TODO verify
  }
}

TEST_CASE("payload-simple-payloadbounds", "[payload][simple][payloadbounds]") {
  SECTION("payload-simple-payloadbounds") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
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
    BlankDevice bd;
    auto p1start = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)}, bd);
    auto p1samestart = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(0)}, bd);
    auto p1end = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date((6 * 60) - 1)}, bd);
    auto p2start = pds.payload(herald::datatype::PayloadTimestamp{.value = herald::datatype::Date(6 * 60)}, bd);
    REQUIRE(p1start == p1samestart);
    REQUIRE(p1start == p1end);
    REQUIRE(p1start != p2start);

  }
}