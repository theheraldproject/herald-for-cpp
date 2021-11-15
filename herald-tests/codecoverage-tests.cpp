//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"


TEST_CASE("coverage-catch-all", "[coverage][catch-all]") {
  SECTION("coverage-catch-all") {
    using namespace herald::datatype;

    // TODO Instantiate all templates to ensure GCov picks them up
    

    Base64String myString;

    REQUIRE("wibble" != myString.encoded());
  }
}