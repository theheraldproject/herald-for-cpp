//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <fstream>
#include <string>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("distribution-empty", "[distribution][datatype][empty]") {
  SECTION("distribution-empty") {
    herald::datatype::Distribution d;

    REQUIRE(0 == d.count());
    REQUIRE(0.0 == d.mean());
    REQUIRE(0.0 == d.variance());
    REQUIRE(0.0 == d.standardDeviation());
    REQUIRE(0.0 != d.min());
    REQUIRE(0.0 != d.max());
  }
}

TEST_CASE("distribution-single", "[distribution][datatype][single]") {
  SECTION("distribution-single") {
    herald::datatype::Distribution d;

    d.add(5);

    REQUIRE(1 == d.count());
    REQUIRE(5.0 == d.mean());
    REQUIRE(0.0 == d.variance());
    REQUIRE(0.0 == d.standardDeviation());
    REQUIRE(5.0 == d.min());
    REQUIRE(5.0 == d.max());
  }
}

TEST_CASE("distribution-sevens", "[distribution][datatype][sevens]") {
  SECTION("distribution-sevens") {
    herald::datatype::Distribution d(7,9);

    REQUIRE(9 == d.count());
    REQUIRE(7.0 == d.mean());
    REQUIRE(0.0 == d.variance());
    REQUIRE(0.0 == d.standardDeviation());
    REQUIRE(7.0 == d.min());
    REQUIRE(7.0 == d.max());
  }
}

TEST_CASE("distribution-geometric", "[distribution][datatype][geometric]") {
  SECTION("distribution-geometric") {
    herald::datatype::Distribution d;

    d.add(1);
    d.add(2);
    d.add(4);
    d.add(8);
    d.add(16);

    REQUIRE(5 == d.count());
    REQUIRE(6.2 == d.mean());
    REQUIRE(37.2 == d.variance());
    REQUIRE(6.099 < d.standardDeviation());
    REQUIRE(6.1 > d.standardDeviation());
    REQUIRE(1.0 == d.min());
    REQUIRE(16.0 == d.max());
  }
}

TEST_CASE("distribution-add", "[distribution][datatype][add]") {
  SECTION("distribution-add") {
    herald::datatype::Distribution d1;
    herald::datatype::Distribution d2;

    d1.add(1);
    d1.add(2);
    d1.add(4);
    d2.add(8);
    d2.add(16);

    d1.add(d2);

    REQUIRE(5 == d1.count());
    REQUIRE(6.2 == d1.mean());
    // Note - variance varies slightly because we're making up the difference in the add function
    INFO("variance is " << d1.variance());
    REQUIRE(37.199 < d1.variance());
    REQUIRE(37.201 > d1.variance());
    REQUIRE(6.099 < d1.standardDeviation());
    REQUIRE(6.1 > d1.standardDeviation());
    REQUIRE(1.0 == d1.min());
    REQUIRE(16.0 == d1.max());

    std::string about = d1;
    INFO(about);
    REQUIRE("" != about);
  }
}

TEST_CASE("distribution-reset", "[distribution][datatype][reset]") {
  SECTION("distribution-reset") {
    herald::datatype::Distribution d;

    d.add(5);

    d.reset();

    REQUIRE(0 == d.count());
    REQUIRE(0.0 == d.mean());
    REQUIRE(0.0 == d.variance());
    REQUIRE(0.0 == d.standardDeviation());
    REQUIRE(0.0 != d.min());
    REQUIRE(0.0 != d.max());
  }
}
