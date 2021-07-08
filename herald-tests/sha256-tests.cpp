//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("sha256-mutated", "[sha256][mutated]") {
  SECTION("sha256-mutated") {
    herald::datatype::SHA256 sha;
    herald::datatype::Data d; // empty data
    auto hash = sha.digest(d);

    bool allZeros = true;
    std::uint8_t value;
    for (std::size_t i = 0;i < 32;++i) {
      bool readOK = hash.uint8(i, value);
      REQUIRE(readOK);
      allZeros = allZeros && value == std::uint8_t(0);
    }

    REQUIRE(hash.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(!allZeros); // Not all zeros (i.e. hash has an actual value)
  }
}

TEST_CASE("sha256-initialised", "[sha256][initialised]") {
  SECTION("sha256-initialised") {
    herald::datatype::SHA256 sha;
    herald::datatype::Data d(std::byte(0),32); // initialised data
    auto hash = sha.digest(d);

    bool allZeros = true;
    std::uint8_t value;
    for (std::size_t i = 0;i < 32;++i) {
      bool readOK = hash.uint8(i, value);
      REQUIRE(readOK);
      allZeros = allZeros && value == std::uint8_t(0);
    }

    REQUIRE(hash.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(!allZeros); // Not all zeros (i.e. hash has an actual value)
  }
}

#ifdef __ZEPHYR__
TEST_CASE("sha256-single-zero", "[.][zephyronly][sha256][single-zero]") {
  SECTION("sha256-single-zero") {
    herald::datatype::SHA256 sha;
    herald::datatype::Data d(std::byte(0),1); // initialised data
    auto hash = sha.digest(d);

    REQUIRE(hash.size() == 32);
    
    std::string encoded = herald::datatype::Base64String::encode(hash).encoded();
    // hex is: 6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d
    std::string expected("bjQLnP+zepicpUTmu3gKLHiQHT+zNzh2hRGjBhevoB0=");

    REQUIRE(expected == encoded);
  }
}

TEST_CASE("sha256-2048-zeros", "[sha256][2048-zeros]") {
  SECTION("sha256-2048-zeros") {
    herald::datatype::SHA256 sha;
    herald::datatype::Data d(std::byte(0),2048); // initialised data
    auto hash = sha.digest(d);
    
    std::string encoded = herald::datatype::Base64String::encode(hash).encoded();
    // hex is: e5a00aa9991ac8a5ee3109844d84a55583bd20572ad3ffcd42792f3c36b183ad
    std::string expected("5aAKqZkayKXuMQmETYSlVYO9IFcq0//NQnkvPDaxg60=");

    REQUIRE(expected == encoded);
  }
}
#endif

TEST_CASE("sha256-differs", "[sha256][differs]") {
  SECTION("sha256-differs") {
    herald::datatype::SHA256 sha1;
    herald::datatype::Data d1(std::byte(1),6);
    auto hash1 = sha1.digest(d1);
    herald::datatype::SHA256 sha2;
    herald::datatype::Data d2(std::byte(2),6);
    auto hash2 = sha2.digest(d2);

    REQUIRE(hash1.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(hash2.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(hash1 != hash2);
  }
}

#ifdef __ZEPHYR__
TEST_CASE("sha256-same", "[sha256][same]") {
  SECTION("sha256-same") {
    herald::datatype::SHA256 sha1;
    herald::datatype::Data d1(std::byte(1),6);
    auto hash1 = sha1.digest(d1);
    herald::datatype::SHA256 sha2;
    herald::datatype::Data d2(std::byte(1),6);
    auto hash2 = sha2.digest(d2);

    REQUIRE(hash1.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(hash2.size() == 32); // SHA-256 always 32 bytes (256 bits)
    REQUIRE(hash1 == hash2);
  }
}
#endif