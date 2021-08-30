//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

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

TEST_CASE("datatypes-proximity-basics", "[datatypes][proximity][basics]") {
  SECTION("datatypes-proximity-basics") {
    herald::datatype::Proximity p{herald::datatype::ProximityMeasurementUnit::RSSI, 11.0};

    REQUIRE(p.unit == herald::datatype::ProximityMeasurementUnit::RSSI);
    REQUIRE(p.value == 11.0);
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






TEST_CASE("datatypes-placename-basics", "[datatypes][placename][ctor][basics]") {
  SECTION("datatypes-placename-basics") {
    herald::datatype::PlacenameLocationReference plr{"Chesterfield"};
    REQUIRE(plr.description().size() >= 12);
  }
}




TEST_CASE("datatypes-uuid-notblank", "[datatypes][uuid][notblank]") {
  SECTION("datatypes-uuid-notblank") {
    auto serviceUUID = herald::datatype::UUID::fromString("428132af-4746-42d3-801e-4572d65bfd9b");
    // INFO("Service UUID " << std::string(serviceUUID));
    auto blankUUID = herald::datatype::UUID::fromString("");
    // INFO("Blank UUID " << std::string(blankUUID));
    REQUIRE(serviceUUID != blankUUID);
  }
}





// TEST_CASE("datatypes-memory-use","[datatypes][memory]") {

//   SECTION("datatypes-memory-use") {
//     // TODO always output sizes to a CSV report file

//     using namespace herald::datatype;
//     Base64String b64 = Base64String::encode(Data(std::byte(2),8));
//     INFO("Base64String size for 8 chars: " << sizeof(b64));
//     REQUIRE(sizeof(b64) <= 40); // std::string of ~12 chars plus 64 bit size
//     Data d{std::byte(1),32};
//     INFO("Data size for 32 bytes: " << sizeof(d));
//     REQUIRE(sizeof(d) <= 40);

//     // TODO other types here
//   }
// }