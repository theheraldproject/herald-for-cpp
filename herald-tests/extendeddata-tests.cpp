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

#include "catch.hpp" // Only one file should include this. Others should be catch.hpp.

#include "herald/herald.h"


TEST_CASE("payload-extendeddata-empty", "[payload][extendeddata][empty]") {
  SECTION("payload-extendeddata-empty") {
    herald::payload::extended::ConcreteExtendedDataV1 d;
    auto pd = d.payload();

    REQUIRE(!pd.has_value());
  }
}

TEST_CASE("payload-extendeddata-one-section", "[payload][extendeddata][one-section]") {
  SECTION("payload-extendeddata-one-section") {
    herald::payload::extended::ConcreteExtendedDataV1 d;
    d.addSection(herald::payload::extended::ExtendedDataSegmentCodesV1::TextPremises,std::string("Adams Pizza"));
    auto pd = d.payload();

    REQUIRE(pd.has_value());
    REQUIRE(pd->size() == 13); // 1 code + 1 length + 11 characters
  }
}