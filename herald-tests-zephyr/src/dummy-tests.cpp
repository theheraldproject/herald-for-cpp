//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "../../herald-tests/catch.hpp"

TEST_CASE("dummy", "[dummy]") {
  SECTION("dummy") {
    REQUIRE(1 == (2 / 2));
  }
}
