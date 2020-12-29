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


TEST_CASE("payload-beacon-basic", "[payload][beacon][basic]") {
  SECTION("payload-beacon-basic") {
    uint16_t country = 826;
    uint16_t state = 4;
    uint32_t code = 123456;
    herald::payload::extended::ConcreteExtendedDataV1 extended;
    extended.addSection(herald::payload::extended::ExtendedDataSegmentCodesV1::TextPremises,std::string("Adams Pizza"));
    herald::payload::beacon::ConcreteBeaconPayloadDataSupplierV1 pds(
      country,
      state,
      code,
      extended
    );
    auto pd = pds.payload(herald::datatype::PayloadTimestamp(),nullptr);

    REQUIRE(pd.has_value());
    REQUIRE(pd->size() == 22); // 1 version code, 2 country, 2 state, 4 code, 13 extended = 22
  }
}

TEST_CASE("payload-beacon-toconstchar", "[payload][beacon][toconstchar]") {
  SECTION("payload-beacon-toconstchar") {
    uint16_t country = 826;
    uint16_t state = 4;
    uint32_t code = 123456;
    herald::payload::extended::ConcreteExtendedDataV1 extended;
    extended.addSection(herald::payload::extended::ExtendedDataSegmentCodesV1::TextPremises,std::string("Adams Pizza"));
    herald::payload::beacon::ConcreteBeaconPayloadDataSupplierV1 pds(
      country,
      state,
      code,
      extended
    );
    auto pd = pds.payload(herald::datatype::PayloadTimestamp(),nullptr);

    REQUIRE(pd.has_value());
    REQUIRE(pd->size() == 22); // 1 version code, 2 country, 2 state, 4 code, 13 extended = 22

    const char* cc = "lorem ipsum dolar sit amet lorem ipsum dolar sit amet lorem ipsum dolar sit amet";
    const char* value = cc;
    char* newvalue = new char[pd->size()];
    std::size_t i;
    for (i = 0;i < pd->size();i++) {
      newvalue[i] = (char)pd->at(i);
    }
    newvalue[i] = '\0';
    // WARNING - DO NOT USE strlen as it terminates on the first \0 (zero) uint8_t byte/character
    REQUIRE(pd->at(21) == std::byte(newvalue[21]));
    value = newvalue;
    REQUIRE(pd->at(21) == std::byte(value[21]));
  }
}
