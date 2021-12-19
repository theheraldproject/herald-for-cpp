//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("allocatablearray-empty", "[allocatablearray][ctor][empty]") {
  SECTION("allocatablearray-empty") {
    herald::datatype::AllocatableArray<herald::datatype::Data> da;
    REQUIRE(0 == da.size());
    REQUIRE(da.begin() == da.end());
    REQUIRE(da.cbegin() == da.cend());

    const herald::datatype::AllocatableArray<herald::datatype::Data> cda;
    REQUIRE(0 == cda.size());
    // The following methods dont exist if const
    // REQUIRE(cda.begin() == cda.end());
    REQUIRE(cda.cbegin() == cda.cend());
  }
}

TEST_CASE("referencearray-empty", "[referencearray][ctor][empty]") {
  SECTION("referencearray-empty") {
    herald::datatype::ReferenceArray<herald::datatype::Data> da;
    REQUIRE(0 == da.size());
    REQUIRE(da.begin() == da.end());
    REQUIRE(da.cbegin() == da.cend());

    const herald::datatype::ReferenceArray<herald::datatype::Data> cda;
    REQUIRE(0 == cda.size());
    // REQUIRE(cda.begin() == cda.end());
    REQUIRE(cda.cbegin() == cda.cend());
  }
}

TEST_CASE("referencearray-single", "[referencearray][ctor][single]") {
  SECTION("referencearray-single") {
    herald::datatype::ReferenceArray<herald::datatype::Data> da;
    herald::datatype::Data d(std::byte(0x0f),6);
    da.add(d);
    REQUIRE(1 == da.size());
    REQUIRE(da.begin() != da.end());
    REQUIRE(da.cbegin() != da.cend());
  }
}