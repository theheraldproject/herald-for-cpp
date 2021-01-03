//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("ble-database-empty", "[ble][database][ctor][empty]") {
  SECTION("ble-database-empty") {
    herald::ble::ConcreteBLEDatabase db;

    REQUIRE(db.size() == 0);
  }
}