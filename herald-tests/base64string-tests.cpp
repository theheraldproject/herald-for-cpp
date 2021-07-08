//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("datatypes-base64string-expected", "[datatypes][base64string][expected]") {
  SECTION("datatypes-base64string-expected") {
    std::string hex("6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d");
    herald::datatype::Data d = herald::datatype::Data::fromHexEncodedString(hex);

    std::string base64 = herald::datatype::Base64String::encode(d).encoded();
    std::string expected("bjQLnP+zepicpUTmu3gKLHiQHT+zNzh2hRGjBhevoB0=");

    REQUIRE(expected == base64);
  }
}