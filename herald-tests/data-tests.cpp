//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>
#include <iostream>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("datatypes-data-ctor-empty", "[datatypes][data][ctor][empty]") {
  SECTION("datatypes-data-ctor-empty") {
    herald::datatype::Data data;
    REQUIRE(data.size() == 0);
    REQUIRE(data.hexEncodedString() == "");
  }
}

TEST_CASE("datatypes-data-ctor-move", "[datatypes][data][ctor][move]") {
  SECTION("datatypes-data-ctor-move") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data orig{bytes, 4};
    herald::datatype::Data d(std::move(orig));

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));

    REQUIRE(orig.size() == 0);
    REQUIRE(orig.hexEncodedString() == ""); // this will definitely try to 'use' the underlying data store
  }
}

TEST_CASE("datatypes-data-assign-move", "[datatypes][data][assign][move]") {
  SECTION("datatypes-data-assign-move") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data orig{bytes, 4};
    herald::datatype::Data d = std::move(orig);

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));

    REQUIRE(orig.size() == 0);
    REQUIRE(orig.hexEncodedString() == ""); // this will definitely try to 'use' the underlying data store
  }
}

TEST_CASE("datatypes-data-ctor-copy", "[datatypes][data][ctor][copy]") {
  SECTION("datatypes-data-ctor-copy") {
    const uint8_t bytes[] = {0,1,2,3};
    const herald::datatype::Data orig{bytes, 4}; // const to ensure a copy
    herald::datatype::Data d(orig);

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));

    REQUIRE(orig.size() == 4);
  }
}

TEST_CASE("datatypes-data-assign-copy", "[datatypes][data][assign][copy]") {
  SECTION("datatypes-data-assign-copy") {
    const uint8_t bytes[] = {0,1,2,3};
    const herald::datatype::Data orig{bytes, 4}; // const to ensure a copy
    herald::datatype::Data d = orig;

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));

    REQUIRE(orig.size() == 4);
  }
}

TEST_CASE("datatypes-data-from-bytearray", "[datatypes][data][ctor][from-bytearray]") {
  SECTION("datatypes-data-from-bytearray") {
    const std::byte bytes[] = {std::byte(0),std::byte(1),std::byte(2),std::byte(3)};
    herald::datatype::Data d{bytes, 4};

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));
  }
}

TEST_CASE("datatypes-data-from-uint8array", "[datatypes][data][ctor][from-uint8array]") {
  SECTION("datatypes-data-from-uint8array") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data d{bytes, 4};

    std::string hs = d.hexEncodedString();
    INFO("Data: uint8array as hexString: expected: 00010203, got: " << hs);

    REQUIRE(d.size() == 4);
    REQUIRE("00010203" == hs);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));
  }
}

TEST_CASE("datatypes-data-from-vector", "[datatypes][data][from-vector]") {
  SECTION("datatypes-data-from-vector") {
    herald::datatype::Base64String str;
    bool encodeOk = herald::datatype::Base64String::from("d290Y2hh",str);
    herald::datatype::Data data = str.decode();
    REQUIRE(encodeOk);
    const char* result = "wotcha";
    REQUIRE(data.size() == 6);
    REQUIRE(data.at(0) == std::byte('w'));
    REQUIRE(data.at(1) == std::byte('o'));
    REQUIRE(data.at(5) == std::byte('a'));

    data.append(data);
    REQUIRE(data.size() == 12);
    REQUIRE(data.at(0) == std::byte('w'));
    REQUIRE(data.at(5) == std::byte('a'));
    REQUIRE(data.at(6) == std::byte('w'));
    REQUIRE(data.at(11) == std::byte('a'));

    auto d2 = data.subdata(3,6);
    herald::datatype::Base64String expStr;
    bool d2EncodeOk = herald::datatype::Base64String::from("Y2hhd290",expStr); // chawot
    herald::datatype::Data expData = expStr.decode();
    REQUIRE(d2.size() == 6);
    REQUIRE(d2 == expData);
    REQUIRE(d2.at(0) == std::byte('c'));
    REQUIRE(d2.at(5) == std::byte('t'));

    auto d3 = d2.subdata(3);
    REQUIRE(d3.size() == 3);
    REQUIRE(d3.at(0) == std::byte('w'));
    REQUIRE(d3.at(2) == std::byte('t'));
  }
}

TEST_CASE("datatypes-data-ctor-repeat", "[datatypes][data][ctor][repeat]") {
  SECTION("datatypes-data-ctor-repeat") {
    herald::datatype::Data d{std::byte('a'),6};

    REQUIRE(d.size() == 6);
    REQUIRE(d.at(0) == std::byte('a'));
    REQUIRE(d.at(5) == std::byte('a'));
  }
}

TEST_CASE("datatypes-data-ctor-fromhexstring", "[datatypes][data][ctor][fromhexstring]") {
  SECTION("datatypes-data-ctor-fromhexstring") {
    const std::string hex = "00010ff0ffcc";
    herald::datatype::Data d = herald::datatype::Data::fromHexEncodedString(hex);
    const std::string finalhex = d.hexEncodedString();
    INFO("Data: fromHexEncodedString: from: " << hex << ", to: " << finalhex);

    REQUIRE(d.size() == 6);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(std::size_t(std::uint8_t(0x0f)) == std::size_t(15));
    // REQUIRE(std::uint8_t(0x0f) == std::uint8_t('f'));
    REQUIRE(d.at(2) == std::byte(15));
    REQUIRE(d.at(3) == std::byte(240));
    REQUIRE(d.at(4) == std::byte(255));
    REQUIRE(d.at(5) == std::byte(204));
  }
}

// TODO uppercase fex encoding test
// TODO mixedcase fex encoding test
// TODO hex string with odd number of chars (leading 0 removed)
// TODO invalid chars in hex string

TEST_CASE("datatypes-data-append", "[datatypes][data][append]") {
  SECTION("datatypes-data-append") {
    herald::datatype::Data d{std::byte('a'),6}; // 6
    const uint8_t u8 = 23; // 1
    const uint16_t u16 = 65530; // 2
    const uint32_t u32 = 122345; // 4
    const uint64_t u64 = 4295967296; // 8
    const std::string s = "lorem ipsum dolar Sit Amet Consecutor"; // 37
    uint8_t byteArray[] = {0,1,2,3,4,5,6,7,8};
    d.append(u8);
    d.append(u16);
    d.append(u32);
    d.append(u64);
    d.append(s);
    d.append(byteArray, 1, 6);

    REQUIRE(d.size() == 64);
    REQUIRE(d.at(0) == std::byte('a'));
    REQUIRE(d.at(5) == std::byte('a'));
    REQUIRE(d.at(6) == std::byte(uint8_t(23)));
    REQUIRE(d.at(57) == std::byte('r'));
    REQUIRE(d.at(58) == std::byte(1));
    REQUIRE(d.at(63) == std::byte(6));

    // now ensure that byte order is correct
    uint8_t r8 = 0;
    uint16_t r16 = 0;
    uint32_t r32 = 0;
    uint64_t r64 = 0;
    bool r8ok = d.uint8(6,r8);
    bool r16ok = d.uint16(7,r16);
    bool r32ok = d.uint32(9,r32);
    bool r64ok = d.uint64(13,r64);
    REQUIRE(r8ok);
    REQUIRE(r16ok);
    REQUIRE(r32ok);
    REQUIRE(r64ok);
    REQUIRE(r8 == u8);
    REQUIRE(r16 == u16);
    REQUIRE(d.at(9) == std::byte(0xe9));
    REQUIRE(d.at(10) == std::byte(0xdd));
    REQUIRE(d.at(11) == std::byte(0x01));
    REQUIRE(d.at(12) == std::byte(0x00));
    REQUIRE(r32 == u32);
    REQUIRE(d.at(20) == std::byte(0x00));
    REQUIRE(d.at(19) == std::byte(0x00));
    REQUIRE(d.at(18) == std::byte(0x00));
    REQUIRE(d.at(17) == std::byte(0x01));
    REQUIRE(d.at(16) == std::byte(0x00));
    REQUIRE(d.at(15) == std::byte(0x0f));
    REQUIRE(d.at(14) == std::byte(0x42));
    REQUIRE(d.at(13) == std::byte(0x40));
    REQUIRE(r64 == u64);
  }
}

TEST_CASE("datatypes-data-reversed", "[datatypes][data][reversed]") {
  SECTION("datatypes-data-reversed") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data d{bytes, 4};

    REQUIRE(d.size() == 4);
    REQUIRE(d.at(0) == std::byte(0));
    REQUIRE(d.at(1) == std::byte(1));
    REQUIRE(d.at(2) == std::byte(2));
    REQUIRE(d.at(3) == std::byte(3));

    herald::datatype::Data rev = d.reversed();
    REQUIRE(rev.size() == 4);
    REQUIRE(rev.at(0) == std::byte(3));
    REQUIRE(rev.at(1) == std::byte(2));
    REQUIRE(rev.at(2) == std::byte(1));
    REQUIRE(rev.at(3) == std::byte(0));
  }
}

TEST_CASE("datatypes-data-description", "[datatypes][data][description]") {
  SECTION("datatypes-data-description") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data d{bytes, 4};

    std::string hex = d.description();
    INFO("Data: description output: " << hex);
    REQUIRE(hex.size() > 0);
    // NOTE: No requirements on format for this method - DO NOT rely on it
  }
}

TEST_CASE("datatypes-data-hexencodedstring", "[datatypes][data][hexencodedstring]") {
  SECTION("datatypes-data-hexencodedstring") {
    const uint8_t bytes[] = {0,1,2,3};
    herald::datatype::Data d{bytes, 4};

    std::string hex = d.hexEncodedString();
    INFO("Data: hexEncodedString (std::string) output: " << hex);
    REQUIRE(8 == hex.size());
    REQUIRE("00010203" == hex);

    std::string hexrev = d.reversed().hexEncodedString();
    INFO("Data: hexEncodedString (std::string) reversed output: " << hexrev);
    REQUIRE(8 == hexrev.size());
    REQUIRE("03020100" == hexrev);
  }
}



TEST_CASE("datatypes-data-subdata-offset-valid", "[datatypes][data][subdata][offset][valid]") {
  SECTION("datatypes-data-subdata-offset-valid") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(4);
    REQUIRE(s.size() == 4);
    REQUIRE(s.at(0) == std::byte(4));
    REQUIRE(s.at(1) == std::byte(5));
    REQUIRE(s.at(2) == std::byte(6));
    REQUIRE(s.at(3) == std::byte(7));
  }
}

TEST_CASE("datatypes-data-subdata-offset-toohigh", "[datatypes][data][subdata][offset][invalid][toohigh]") {
  SECTION("datatypes-data-subdata-offset-toohigh") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(8);
    REQUIRE(s.size() == 0);
  }
}

TEST_CASE("datatypes-data-subdata-offset-toolow", "[datatypes][data][subdata][offset][invalid][toolow]") {
  SECTION("datatypes-data-subdata-offset-toolow") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(-1); // std::size_t is signed, but -1 makes a very high size, so should be tested
    REQUIRE(s.size() == 0);
  }
}



TEST_CASE("datatypes-data-subdata-length-valid", "[datatypes][data][subdata][length][valid]") {
  SECTION("datatypes-data-subdata-length-valid") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(4, 3);
    REQUIRE(s.size() == 3);
    REQUIRE(s.at(0) == std::byte(4));
    REQUIRE(s.at(1) == std::byte(5));
    REQUIRE(s.at(2) == std::byte(6));
  }
}

TEST_CASE("datatypes-data-subdata-length-toohigh", "[datatypes][data][subdata][length][toohigh]") {
  SECTION("datatypes-data-subdata-length-toohigh") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(4, 5);
    REQUIRE(s.size() == 4);
    REQUIRE(s.at(0) == std::byte(4));
    REQUIRE(s.at(1) == std::byte(5));
    REQUIRE(s.at(2) == std::byte(6));
    REQUIRE(s.at(3) == std::byte(7));
  }
}

TEST_CASE("datatypes-data-subdata-length-toolow", "[datatypes][data][subdata][length][toolow]") {
  SECTION("datatypes-data-subdata-length-toolow") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(4, -1); // std::size_t is signed, but -1 makes a very high size, so should be tested
    REQUIRE(s.size() == 4); // no effective way to tell the difference between this and too high length
    REQUIRE(s.at(0) == std::byte(4));
    REQUIRE(s.at(1) == std::byte(5));
    REQUIRE(s.at(2) == std::byte(6));
    REQUIRE(s.at(3) == std::byte(7));
  }
}

TEST_CASE("datatypes-data-subdata-length-zero", "[datatypes][data][subdata][length][zero]") {
  SECTION("datatypes-data-subdata-length-zero") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    herald::datatype::Data s = d.subdata(4, 0); 
    REQUIRE(s.size() == 0);
  }
}



TEST_CASE("datatypes-data-at-valid", "[datatypes][data][at][valid]") {
  SECTION("datatypes-data-at-valid") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    REQUIRE(d.at(3) == std::byte(3));
  }
}

TEST_CASE("datatypes-data-at-toohigh", "[datatypes][data][at][invalid][toohigh") {
  SECTION("datatypes-data-at-toohigh") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    REQUIRE(d.at(8) == std::byte(0));
  }
}

TEST_CASE("datatypes-data-at-toolow", "[datatypes][data][at][invalid][toolow") {
  SECTION("datatypes-data-at-toolow") {
    const uint8_t bytes[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d{bytes, 8};

    REQUIRE(d.at(-1) == std::byte(0)); // -1 and LONG_LONG_MAX are the same, so should be same as toohigh
  }
}


TEST_CASE("datatypes-data-extract-valid", "[datatypes][data][extract][valid]") {
  SECTION("datatypes-data-extract-valid") {
    // REMEMBER Data uses little endian for encoding/decoding of individual types
    // OUTER byte order big endian, data byte order little endian
    const uint8_t bytes[] = { 0, 1, 
      0x7C, 0x02, // 636 as unsigned int16 in hex
      0x89, 0x90,  // 37001 as usigned int16
      0xb3, 0xb5, 0x56, 0x07,  // 123123123 as uint32 // NB last byte also used in uint64 decoding
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
      0xff // extra data at end to ensure uint64 is not off by one
    };
    herald::datatype::Data d{bytes, 18};

    uint8_t u8 = 0;
    bool u8ok = d.uint8(0, u8);
    REQUIRE(u8ok);
    REQUIRE(u8 == 0x00);

    u8ok = d.uint8(1, u8);
    REQUIRE(u8ok);
    REQUIRE(u8 == 0x01);

    u8ok = d.uint8(17, u8);
    REQUIRE(u8ok);
    REQUIRE(u8 == 0xff);

    uint16_t u16 = 0;
    bool u16ok = d.uint16(2, u16);
    REQUIRE(u16ok);
    REQUIRE(u16 == 636);

    u16 = 0;
    u16ok = d.uint16(4, u16);
    REQUIRE(u16ok);
    REQUIRE(u16 == 37001);

    uint32_t u32 = 0;
    bool u32ok = d.uint32(6, u32);
    REQUIRE(u32ok);
    REQUIRE(u32 == 123123123);

    uint64_t u64 = 0;
    bool u64ok = d.uint64(9, u64);
    REQUIRE(u64ok);
    REQUIRE(u64 == 0xffffffffffffff07);
  }
}


TEST_CASE("datatypes-data-append-bytearray-valid", "[datatypes][data][append][bytearray][valid]") {
  SECTION("datatypes-data-append-bytearray-valid") {
    uint8_t initial[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d(initial, 8);
    uint8_t byteArray[] = {0,1,2,3,4,5,6,7};

    d.append(byteArray,0,8);
    REQUIRE(d.size() == 16);
    REQUIRE(d.at(8) == std::byte(0));
    REQUIRE(d.at(15) == std::byte(7));
  }
}


TEST_CASE("datatypes-data-append-data-valid", "[datatypes][data][append][data][valid]") {
  SECTION("datatypes-data-append-data-valid") {
    uint8_t initial[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d(initial, 8);
    uint8_t byteArray[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data more(byteArray,8);

    d.append(more);
    REQUIRE(d.size() == 16);
    REQUIRE(d.at(8) == std::byte(0));
    REQUIRE(d.at(15) == std::byte(7));
  }
}


TEST_CASE("datatypes-data-append-data-reversed-valid", "[datatypes][data][append][data][reversed][valid]") {
  SECTION("datatypes-data-append-data-reversed-valid") {
    uint8_t initial[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d(initial, 8);
    uint8_t byteArray[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data more(byteArray,8);

    d.append(more.reversed());
    REQUIRE(d.size() == 16);
    REQUIRE(d.at(8) == std::byte(7));
    REQUIRE(d.at(15) == std::byte(0));
  }
}


TEST_CASE("datatypes-data-append-data-reversed-mthd-valid", "[datatypes][data][append][data][reversed][mthd][valid]") {
  SECTION("datatypes-data-append-data-reversed-mthd-valid") {
    uint8_t initial[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d(initial, 8);
    uint8_t byteArray[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data more(byteArray,8);

    d.appendReversed(more,0,more.size());
    REQUIRE(d.size() == 16);
    REQUIRE(d.at(8) == std::byte(7));
    REQUIRE(d.at(15) == std::byte(0));
  }
}



TEST_CASE("datatypes-data-equals", "[datatypes][data][equals]") {
  SECTION("datatypes-data-equals") {
    uint8_t initial[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d1(initial, 8);

    uint8_t byteArray[] = {0,1,2,3,4,5,6,7};
    herald::datatype::Data d2(byteArray,8);
    uint8_t byteArray2[] = {4,4,4,4,4,4,4,4};
    herald::datatype::Data d3(byteArray2,8);

    REQUIRE(d1.size() == d2.size());
    REQUIRE(d1.at(0) == d2.at(0));
    REQUIRE(d1.at(7) == d2.at(7));
    REQUIRE(d1.hashCode() == d2.hashCode());
    REQUIRE(false == (d2.hashCode() == d3.hashCode())); // tests operator==
    REQUIRE(d2.hashCode() != d3.hashCode());
    REQUIRE(d3.hashCode() != d1.hashCode());
    REQUIRE(d1 == d2);
    REQUIRE(false == (d1 == d3)); // tests operator==
    REQUIRE(d1 != d3); // tests operator!=
    REQUIRE(d2 != d3);
    REQUIRE(d3 != d1);

  }
}
