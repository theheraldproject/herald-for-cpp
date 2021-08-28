//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

TEST_CASE("payload-fixed-basic", "[payload][fixed][basic]") {
  SECTION("payload-fixed-basic") {
    std::uint16_t country = 826;
    std::uint16_t state = 4;
    std::uint64_t clientId = 123456789;
    herald::payload::fixed::ConcreteFixedPayloadDataSupplierV1 pds(
      country,
      state,
      clientId
    );
    BlankDevice bd;
    auto pd = pds.payload(herald::datatype::PayloadTimestamp(),bd);

    REQUIRE(pd.size() == 13); // 1 version code, 2 country, 2 state, 8 clientId = 13
    std::uint8_t rpidversion = 0;
    std::uint16_t rc = 0;
    std::uint16_t rs = 0;
    std::uint64_t rcid = 0;
    REQUIRE(pd.uint8(0,rpidversion));
    REQUIRE(pd.uint16(1,rc));
    REQUIRE(pd.uint16(3,rs));
    REQUIRE(pd.uint64(5,rcid));
    REQUIRE(rpidversion == std::uint8_t(0x08));
    REQUIRE(rc == country);
    REQUIRE(rs == state);
    REQUIRE(rcid == clientId);
  }
}
