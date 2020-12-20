//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

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



