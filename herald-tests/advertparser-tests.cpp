//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("advert-parser-subdatabigendian", "[advert][parser][subdatabigendian]") {
  SECTION("advert-parser-subdatabigendian") {
    std::uint8_t data[] {0,1,5,6,7,8,12,13,14};
    herald::datatype::Data original(data,9);
    REQUIRE(std::byte(5) == original.at(2));
    REQUIRE(std::byte(6) == original.at(3));
    REQUIRE(std::byte(7) == original.at(4));
    REQUIRE(std::byte(8) == original.at(5));
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,2,4);
    REQUIRE(result.size() == 4);
    REQUIRE(std::byte(5) == result.at(0));
    REQUIRE(std::byte(6) == result.at(1));
    REQUIRE(std::byte(7) == result.at(2));
    REQUIRE(std::byte(8) == result.at(3));
  }
}
TEST_CASE("advert-parser-subdatalittleendian", "[advert][parser][subdatalittleendian]") {
  SECTION("advert-parser-subdatalittleendian") {
    std::uint8_t data[] {0,1,5,6,7,8,12,13,14};
    herald::datatype::Data original(data, 9);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,2,4);
    REQUIRE(result.size() == 4);
    REQUIRE(std::byte(8) == result.at(0));
    REQUIRE(std::byte(7) == result.at(1));
    REQUIRE(std::byte(6) == result.at(2));
    REQUIRE(std::byte(5) == result.at(3));
  }
}

TEST_CASE("advert-parser-subdatabigendianoverflow", "[advert][parser][subdatabigendianoverflow]") {
  SECTION("advert-parser-subdatabigendianoverflow") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,2,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatalittleendianoverflow", "[advert][parser][subdatalittleendianoverflow]") {
  SECTION("advert-parser-subdatalittleendianoverflow") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,2,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatabigendianlowindex", "[advert][parser][subdatabigendianlowindex]") {
  SECTION("advert-parser-subdatabigendianlowindex") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,-1,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatalittleendianlowindex", "[advert][parser][subdatalittleendianlowindex]") {
  SECTION("advert-parser-subdatalittleendianlowindex") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,-1,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatabigendianhighindex", "[advert][parser][subdatabigendianhighindex]") {
  SECTION("advert-parser-subdatabigendianhighindex") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,5,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatalittleendianhighindex", "[advert][parser][subdatalittleendianhighindex]") {
  SECTION("advert-parser-subdatalittleendianhighindex") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,5,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatabigendianlargelength", "[advert][parser][subdatabigendianlargelength]") {
  SECTION("advert-parser-subdatabigendianlargelength") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,2,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatalittleendianlargelength", "[advert][parser][subdatalittleendianlargelength]") {
  SECTION("advert-parser-subdatalittleendianlargelength") {
    std::uint8_t data[] {0,1,5,6,7};
    herald::datatype::Data original(data, 5);
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,2,4);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatabigendianemptydata", "[advert][parser][subdatabigendianemptydata]") {
  SECTION("advert-parser-subdatabigendianemptydata") {
    herald::datatype::Data original;
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataBigEndian(original,0,1);
    REQUIRE(0 == result.size());
  }
}

TEST_CASE("advert-parser-subdatalittleendianemptydata", "[advert][parser][subdatalittleendianemptydata]") {
  SECTION("advert-parser-subdatalittleendianemptydata") {
    herald::datatype::Data original;
    herald::datatype::Data result = 
      herald::ble::filter::BLEAdvertParser::subDataLittleEndian(original,0,1);
    REQUIRE(0 == result.size());
  }
}
/*

// TODO MOVE THESE TO DATA ITSELF
    @Test
    public void testDataSubsetBigEndianNullData() throws Exception {
        byte[] result = BLEAdvertParser.subDataBigEndian(null,0,1);
        assertNotNull(result);
        assertEquals(0, result.length);
    }

    @Test
    public void testDataSubsetLittleNullEmptyData() throws Exception {
        byte[] result = BLEAdvertParser.subDataLittleEndian(null,0,1);
        assertNotNull(result);
        assertEquals(0, result.length);
    }
*/

// MARK: HIGH LEVEL FULL PACKET METHODS

TEST_CASE("advert-parser-appletvfg", "[advert][parser][appletvfg]") {
  SECTION("advert-parser-appletvfg") {
    std::uint8_t data[] {
      0x02, 0x01, 0x1a, 
      0x02, 0x0a, 0x08,
      0x0c, 0xff, 0x4c, 0x00,
      0x10, 0x07, 0x33,
      0x1f, 0x2c, 0x30, 0x2f, 0x92,
      0x58
    };
    herald::datatype::Data original(data, 19);
    REQUIRE(19 == original.size());
    REQUIRE("02011a020a080cff4c001007331f2c302f9258" == original.hexEncodedString());
  
    auto result = herald::ble::filter::BLEAdvertParser::extractSegments(original,0);
    REQUIRE(3 == result.size());
    REQUIRE(result[0].type == herald::ble::filter::BLEAdvertSegmentType::flags);
    REQUIRE(result[1].type == herald::ble::filter::BLEAdvertSegmentType::txPowerLevel);
    REQUIRE(result[2].type == herald::ble::filter::BLEAdvertSegmentType::manufacturerData);

    auto manu = herald::ble::filter::BLEAdvertParser::extractManufacturerData(result);
    REQUIRE(1 == manu.size());

    auto manuData = manu.front();
    REQUIRE(9 == manuData.data.size());
    REQUIRE(std::byte(0x10) == manuData.data.at(0));
    REQUIRE(std::byte(0x07) == manuData.data.at(1));

    // Check we get an apple manufacturer data from it
    auto appleData = herald::ble::filter::BLEAdvertParser::extractAppleManufacturerSegments(manu);
    REQUIRE(1 == appleData.size());
    auto appleMD = appleData.front();
    REQUIRE(appleMD.data.size() == 7); // not including initial type code, and length value
    REQUIRE(appleMD.type == 0x10);
  }
}

TEST_CASE("advert-parser-heraldpseudoaddress", "[advert][parser][heraldpseudoaddress]") {
  SECTION("advert-parser-heraldpseudoaddress") {
    std::uint8_t data[] {
      0x02, 0x01, 0x1a, 
      0x02, 0x0a, 0x08,
      0x09, 0xff, 0xff, 0xfa,
      0x10, 0x07, 0x33, 0x1f, 0x2c, 0x30
    };
    herald::datatype::Data original(data, 16);
    REQUIRE(16 == original.size());
    REQUIRE("02011a020a0809fffffa1007331f2c30" == original.hexEncodedString());
  
    auto result = herald::ble::filter::BLEAdvertParser::extractSegments(original,0);
    REQUIRE(3 == result.size());
    REQUIRE(result[0].type == herald::ble::filter::BLEAdvertSegmentType::flags);
    REQUIRE(result[1].type == herald::ble::filter::BLEAdvertSegmentType::txPowerLevel);
    REQUIRE(result[2].type == herald::ble::filter::BLEAdvertSegmentType::manufacturerData);

    auto manu = herald::ble::filter::BLEAdvertParser::extractManufacturerData(result);
    REQUIRE(1 == manu.size());

    auto manuData = manu.front();
    REQUIRE(6 == manuData.data.size());
    REQUIRE(std::byte(0x10) == manuData.data.at(0));
    REQUIRE(std::byte(0x07) == manuData.data.at(1));

    // Check we get a herald manufacturer data from it
    auto heraldDataV = herald::ble::filter::BLEAdvertParser::extractHeraldManufacturerData(manu);
    REQUIRE(heraldDataV.size() == 1);
    auto heraldData = heraldDataV.front();
    REQUIRE(6 == heraldData.size());
    REQUIRE(std::byte(0x10) == heraldData.at(0));
    REQUIRE(std::byte(0x30) == heraldData.at(5));
  }
}
