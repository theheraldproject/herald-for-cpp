//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>
#include <iostream>

#include "catch.hpp"

#include "herald/herald.h"

/**
 * Implementation note: BLE Mac should ALWAYS be valid 6 bytes, even if data wrong
 */

TEST_CASE("ble-macaddress-basic", "[ble][macaddress][basic]") {
  SECTION("ble-macaddress-basic") {
    std::uint8_t data[] {0,1,2,3,4,5}; // mac address data
    herald::ble::BLEMacAddress mac(data);
    herald::datatype::Data original = (herald::datatype::Data)mac; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(1) == original.at(1));
    REQUIRE(std::byte(2) == original.at(2));
    REQUIRE(std::byte(3) == original.at(3));
    REQUIRE(std::byte(4) == original.at(4));
    REQUIRE(std::byte(5) == original.at(5));
  }
}

TEST_CASE("ble-macaddress-ctor-copy", "[ble][macaddress][ctor][copy") {
  SECTION("ble-macaddress-ctor-copy") {
    std::uint8_t data[] {0,1,2,3,4,5}; // mac address data
    herald::ble::BLEMacAddress mac(data);
    herald::ble::BLEMacAddress maccopy(mac);
    herald::datatype::Data original = (herald::datatype::Data)maccopy; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(1) == original.at(1));
    REQUIRE(std::byte(2) == original.at(2));
    REQUIRE(std::byte(3) == original.at(3));
    REQUIRE(std::byte(4) == original.at(4));
    REQUIRE(std::byte(5) == original.at(5));
    REQUIRE(mac == maccopy);
    herald::datatype::Data originalOrig = (herald::datatype::Data)mac;
    REQUIRE(original == originalOrig); 
  }
}

TEST_CASE("ble-macaddress-empty", "[ble][macaddress][empty]") {
  SECTION("ble-macaddress-empty") {
    herald::ble::BLEMacAddress mac;
    herald::datatype::Data original = (herald::datatype::Data)mac; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(0) == original.at(1));
    REQUIRE(std::byte(0) == original.at(2));
    REQUIRE(std::byte(0) == original.at(3));
    REQUIRE(std::byte(0) == original.at(4));
    REQUIRE(std::byte(0) == original.at(5));
  }
}

TEST_CASE("ble-macaddress-emptydata", "[ble][macaddress][emptydata]") {
  SECTION("ble-macaddress-emptydata") {
    herald::ble::Data d;
    herald::ble::BLEMacAddress mac(d);
    herald::datatype::Data original = (herald::datatype::Data)mac; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(0) == original.at(1));
    REQUIRE(std::byte(0) == original.at(2));
    REQUIRE(std::byte(0) == original.at(3));
    REQUIRE(std::byte(0) == original.at(4));
    REQUIRE(std::byte(0) == original.at(5));
  }
}

TEST_CASE("ble-macaddress-smalldata", "[ble][macaddress][smalldata]") {
  SECTION("ble-macaddress-smalldata") {
    std::uint8_t data[] {0,1,2,3}; // mac address data - too small! Use what data we can
    herald::ble::Data d(data,4);
    herald::ble::BLEMacAddress mac(d);
    herald::datatype::Data original = (herald::datatype::Data)mac; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(1) == original.at(1));
    REQUIRE(std::byte(2) == original.at(2));
    REQUIRE(std::byte(3) == original.at(3));
    REQUIRE(std::byte(0) == original.at(4)); // last two empty
    REQUIRE(std::byte(0) == original.at(5)); // last two empty
  }
}

TEST_CASE("ble-macaddress-largedata", "[ble][macaddress][largedata]") {
  SECTION("ble-macaddress-largedata") {
    std::uint8_t data[] {0,1,2,3,4,5,6,7,8,9,10,11}; // mac address data - too large!
    herald::ble::Data d(data,12);
    herald::ble::BLEMacAddress mac(d);
    herald::datatype::Data original = (herald::datatype::Data)mac; // explicit conversion operator test
    REQUIRE(6 == original.size());
    REQUIRE(std::byte(0) == original.at(0));
    REQUIRE(std::byte(1) == original.at(1));
    REQUIRE(std::byte(2) == original.at(2));
    REQUIRE(std::byte(3) == original.at(3));
    REQUIRE(std::byte(4) == original.at(4));
    REQUIRE(std::byte(5) == original.at(5));
  }
}

TEST_CASE("ble-macaddress-tostring", "[ble][macaddress][tostring]") {
  SECTION("ble-macaddress-tostring") {
    std::uint8_t data[] {0,1,2,3,4,5}; // mac address data
    herald::ble::Data d(data,6);
    herald::ble::BLEMacAddress mac(d);
    std::string description = (std::string)mac; // conversion operator
    INFO("BLEMacAddress: String description: " << description);
    REQUIRE(17 == description.size()); // 2 chars per 6 data elements, 5 : char separators
    REQUIRE("05:04:03:02:01:00" == description); // little endian conversion
  }
}