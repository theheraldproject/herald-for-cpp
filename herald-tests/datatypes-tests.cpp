//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

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

#include "tests.h" // Only one file should include this. Others should be catch.hpp.

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
