//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"


TEST_CASE("datatypes-payloaddata-basics", "[datatypes][payloaddata][basics]") {
  SECTION("datatypes-payloaddata-basics") {
    herald::datatype::PayloadData payload{}; // empty default ctor

    REQUIRE(payload.size() == 0);

    
    herald::datatype::PayloadData payload2{std::byte('a'), 6}; // repeating byte ctor

    REQUIRE(payload2.size() == 6);
    REQUIRE(payload2.at(0) == std::byte('a'));
    REQUIRE(payload2.at(5) == std::byte('a'));

    const char* charArray = "wotcha";
    std::byte byteArray[6];
    for (int i = 0;i < 6;i++) {
      byteArray[i] = std::byte(charArray[i]);
    }
    herald::datatype::PayloadData payload3{byteArray, 4};

    REQUIRE(payload3.size() == 4);
    REQUIRE(payload3.at(0) == std::byte('w'));
    REQUIRE(payload3.at(3) == std::byte('c'));
  }
}



TEST_CASE("datatypes-immediatesenddata-basics", "[datatypes][immediatesenddata][basics]") {
  SECTION("datatypes-immediatesenddata-basics") {
    herald::datatype::Data d{std::byte('f'),8};
    herald::datatype::ImmediateSendData isd(d);

    REQUIRE(isd.size() == 8);
    REQUIRE(isd.at(0) == std::byte('f'));
    REQUIRE(isd.at(7) == std::byte('f'));
  }
}


TEST_CASE("datatypes-payloadsharingdata-basics", "[datatypes][payloadsharingdata][ctor][basics]") {
  SECTION("datatypes-payloadsharingdata-basics") {
    herald::datatype::PayloadSharingData psd{{11},{std::byte('g'),4}};

    REQUIRE(psd.rssi.intValue() == 11);
    REQUIRE(psd.data.size() == 4);
    REQUIRE(psd.data.at(3) == std::byte('g'));
  }
}

