//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "tests.h" // Only one file should include this. Others should be catch.hpp.

#include "herald/herald.h"


TEST_CASE("datatypes-base64string-reversible", "[datatypes][base64string][reversible]") {
  // Story:-
  //   [Who]   As an app programmer
  //   [What]  I need to encode and decode in base64
  //   [Value] So I have a reliable string based way of exchanging information
  SECTION("datatypes-base64string-reversible") {
    herald::datatype::Base64String str;
    bool encodeOk = herald::datatype::Base64String::from("d290Y2hh",str);
    REQUIRE(encodeOk);
    REQUIRE(str.encoded() == "d290Y2hh");
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

TEST_CASE("datatypes-data-append", "[datatypes][data][append]") {
  SECTION("datatypes-data-append") {
    herald::datatype::Data d{std::byte('a'),6}; // 6
    const uint8_t u8 = 23; // 1
    const uint16_t u16 = 65530; // 2
    const uint32_t u32 = 122345; // 4
    const uint64_t u64 = 4295967296; // 8
    const std::string s = "lorem ipsum dolar Sit Amet Consecutor"; // 37
    d.append(u8);
    d.append(u16);
    d.append(u32);
    d.append(u64);
    d.append(s);

    REQUIRE(d.size() == 58);
    REQUIRE(d.at(0) == std::byte('a'));
    REQUIRE(d.at(5) == std::byte('a'));
    REQUIRE(d.at(6) == std::byte(uint8_t(23)));
    REQUIRE(d.at(57) == std::byte('r'));

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

TEST_CASE("datatypes-date-basics", "[datatypes][date][basics]") {
  SECTION("datatypes-date-basics") {
    herald::datatype::Date d(1608483600); // long ctor

    REQUIRE(d.secondsSinceUnixEpoch() == 1608483600);
    REQUIRE(d.iso8601DateTime() == std::string("2020-12-20T17:00:00Z"));
    REQUIRE(d.toString() == std::string("2020-12-20T17:00:00Z"));

    herald::datatype::Date d2(d); // copy ctor
    REQUIRE(d2.secondsSinceUnixEpoch() == 1608483600);
    REQUIRE(d2.iso8601DateTime() == std::string("2020-12-20T17:00:00Z"));
    REQUIRE(d2.toString() == std::string("2020-12-20T17:00:00Z"));

    // TODO Default constructor producing 'now'
  }
}

TEST_CASE("datatypes-proximity-basics", "[datatypes][proximity][basics]") {
  SECTION("datatypes-proximity-basics") {
    herald::datatype::Proximity p{herald::datatype::ProximityMeasurementUnit::RSSI, 11.0};

    REQUIRE(p.unit == herald::datatype::ProximityMeasurementUnit::RSSI);
    REQUIRE(p.value == 11.0);
  }
}



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






TEST_CASE("datatypes-encounter-basics", "[datatypes][encounter][basics]") {
  SECTION("datatypes-encounter-basics") {
    herald::datatype::Proximity prox{herald::datatype::ProximityMeasurementUnit::RSSI, 11.0};
    herald::datatype::PayloadData payload{std::byte('b'),6};
    herald::datatype::Date date{1608483600};
    herald::datatype::Encounter e{prox,payload,date}; // ctor

    REQUIRE(e.isValid() == true);
    REQUIRE(e.timestamp().secondsSinceUnixEpoch() == 1608483600);
    REQUIRE(e.proximity().unit == herald::datatype::ProximityMeasurementUnit::RSSI);
    REQUIRE(e.proximity().value == 11.0);
    REQUIRE(e.payload().size() == 6);
  }
}

// TODO Encounter to csvString format test





TEST_CASE("datatypes-immediatesenddata-basics", "[datatypes][immediatesenddata][basics]") {
  SECTION("datatypes-immediatesenddata-basics") {
    herald::datatype::Data d{std::byte('f'),8};
    herald::datatype::ImmediateSendData isd(d);

    REQUIRE(isd.size() == 8);
    REQUIRE(isd.at(0) == std::byte('f'));
    REQUIRE(isd.at(7) == std::byte('f'));
  }
}




TEST_CASE("datatypes-errorcode-ctor-default", "[datatypes][errorcode][ctor-default]") {
  SECTION("datatypes-errorcode-ctor-default") {
    herald::datatype::ErrorCode ec;
    REQUIRE(ec()); // bool conversion to true
    REQUIRE(ec.message() == std::string(""));
  }
}


TEST_CASE("datatypes-errorcode-ctor-bool", "[datatypes][errorcode][ctor-bool]") {
  SECTION("datatypes-errorcode-ctor-bool-true") {
    herald::datatype::ErrorCode ec(true);
    REQUIRE(ec()); // bool conversion to true
    REQUIRE(ec.message() == std::string(""));
  }
  SECTION("datatypes-errorcode-ctor-bool-false") {
    herald::datatype::ErrorCode ec(false);
    REQUIRE(!ec()); // bool conversion to true
    REQUIRE(ec.message() == std::string(""));
  }
}

TEST_CASE("datatypes-errorcode-ctor-bool-string", "[datatypes][errorcode][ctor-bool-string]") {
  SECTION("datatypes-errorcode-ctor-bool-string-true") {
    herald::datatype::ErrorCode ec(true, "wibble");
    REQUIRE(ec()); // bool conversion to true
    REQUIRE(ec.message() == std::string("wibble"));
  }
  SECTION("datatypes-errorcode-ctor-bool-string-false") {
    herald::datatype::ErrorCode ec(false, "wibble");
    REQUIRE(!ec()); // bool conversion to true
    REQUIRE(ec.message() == std::string("wibble"));
  }
}


TEST_CASE("datatypes-rssi-ctor-int", "[datatypes][rssi][ctor][int]") {
  SECTION("datatypes-rssi-ctor-int") {
    herald::datatype::RSSI rssi{11};
    REQUIRE(rssi.intValue() == 11);

    herald::datatype::RSSI other{11};
    REQUIRE(other == rssi);
  }
}



TEST_CASE("datatypes-rssi-ctor-copy", "[datatypes][rssi][ctor][copy]") {
  SECTION("datatypes-rssi-ctor-copy") {
    herald::datatype::RSSI rssi{11};
    REQUIRE(rssi.intValue() == 11);

    herald::datatype::RSSI other{rssi};
    REQUIRE(other == rssi);
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





TEST_CASE("datatypes-placename-basics", "[datatypes][placename][ctor][basics]") {
  SECTION("datatypes-placename-basics") {
    herald::datatype::PlacenameLocationReference plr{"Chesterfield"};
    REQUIRE(plr.description().size() >= 12);
  }
}






TEST_CASE("datatypes-targetidentifier-ctor-default", "[datatypes][targetidentifier][ctor][default]") {
  SECTION("datatypes-targetidentifier-ctor-default") {
    herald::datatype::TargetIdentifier t1;

    REQUIRE(t1.toString().size() == 0);
  }
}

// TODO Once we have the local BluetoothReference / MAC equivalent, add ctor tests here





TEST_CASE("datatypes-timeinterval-basics", "[datatypes][timeinterval][ctor][basics]") {
  SECTION("datatypes-timeinterval-basics") {
    herald::datatype::TimeInterval ti{1200};

    REQUIRE(ti.millis() == 1'200'000);

    auto t2 = herald::datatype::TimeInterval::never();
    REQUIRE(t2.millis() == LONG_MAX);
    REQUIRE(t2.toString() == std::string("never"));

    auto t3 = herald::datatype::TimeInterval::minutes(20);
    REQUIRE(t3.millis() == 20 * 60 * 1000);

    auto t4 = herald::datatype::TimeInterval::seconds(20);
    REQUIRE(t4.millis() == 20 * 1000);

    herald::datatype::Date d1{1000};
    herald::datatype::Date d2{1200};
    herald::datatype::TimeInterval t5(d1,d2);

    REQUIRE(t5.millis() == 200 * 1000);
    REQUIRE(t5.toString() == std::string("200"));
  }
}





// TEST_CASE("datatypes-uuid-basics", "[datatypes][uuid][ctor][basics]") {
//   SECTION("datatypes-uuid-basics") {
//     auto uuid1 = herald::datatype::UUID::random();
//     REQUIRE(uuid1.valid());
//     REQUIRE(uuid1.string().size() == 36); // 4 hyphens, 16 hex bytes = 36 characters
//   }
// }


TEST_CASE("random-allzeros","[randomness][allzeros][basic][datatypes]") {

  SECTION("random-allzeros-basic") {
    herald::datatype::AllZerosNotRandom rnd;
    REQUIRE(rnd.nextInt() == 0);
    REQUIRE(rnd.nextDouble() == 0);
    std::vector<std::byte> zeros;
    for (int i = 0;i < 4;i++) {
      zeros.push_back(std::byte(0));
    }
    herald::datatype::Data expected(zeros);
    herald::datatype::Data toFill;
    rnd.nextBytes(4, toFill);
    REQUIRE(toFill == expected);
  }
}




TEST_CASE("datatypes-uuid-notrandom","[randomness][uuid][basic][datatypes]") {

  SECTION("datatypes-uuid-notrandom") {
    std::unique_ptr<herald::datatype::AllZerosNotRandom> rnd = std::make_unique<herald::datatype::AllZerosNotRandom>();
    herald::datatype::RandomnessGenerator gen(std::move(rnd));
    auto emptyV4 = herald::datatype::UUID::random(gen);
    REQUIRE(emptyV4.string() == std::string("00000000-0000-4000-8000-000000000000")); // v4 variant 1
  }
}




TEST_CASE("datatypes-uuid-random","[randomness][uuid][basic][datatypes]") {

  SECTION("datatypes-uuid-random") {
    std::unique_ptr<herald::datatype::IntegerDistributedRandomSource> rnd = 
      std::make_unique<herald::datatype::IntegerDistributedRandomSource>();
    herald::datatype::RandomnessGenerator gen(std::move(rnd));
    auto randomV4 = herald::datatype::UUID::random(gen);
    std::string str = randomV4.string();
    std::cout << "UUID v4 random value: " << str << std::endl;
    REQUIRE(str != std::string("00000000-0000-4000-8000-000000000000")); // v4 variant 1
  }
}