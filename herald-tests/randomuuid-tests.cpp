//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>

#include "catch.hpp"

#include "herald/herald.h"


// TEST_CASE("datatypes-uuid-basics", "[datatypes][uuid][ctor][basics]") {
//   SECTION("datatypes-uuid-basics") {
//     auto uuid1 = herald::datatype::UUID::random();
//     REQUIRE(uuid1.valid());
//     REQUIRE(uuid1.string().size() == 36); // 4 hyphens, 16 hex bytes = 36 characters
//   }
// }


TEST_CASE("random-allzeros","[randomness][allzeros][basic][datatypes]") {

  SECTION("random-allzeros-basic") {
    herald::datatype::AllZerosNotRandom rnd;
    REQUIRE(rnd.nextInt() == 0);
    REQUIRE(rnd.nextDouble() == 0);
    herald::datatype::Data expected(std::byte(0),4);
    herald::datatype::Data toFill;
    rnd.nextBytes(4, toFill);
    REQUIRE(toFill == expected);
  }
}




TEST_CASE("datatypes-uuid-notrandom","[randomness][uuid][basic][datatypes]") {

  SECTION("datatypes-uuid-notrandom") {
    std::unique_ptr<herald::datatype::AllZerosNotRandom> rnd = std::make_unique<herald::datatype::AllZerosNotRandom>();
    herald::datatype::RandomnessGenerator gen(std::move(rnd));
    auto emptyV4 = herald::datatype::UUID::random(gen);
    REQUIRE(emptyV4.string() == std::string("00000000-0000-4000-8000-000000000000")); // v4 variant 1
  }
}




TEST_CASE("datatypes-uuid-random","[randomness][uuid][basic][datatypes]") {

  SECTION("datatypes-uuid-random") {
    std::unique_ptr<herald::datatype::IntegerDistributedRandomSource> rnd = 
      std::make_unique<herald::datatype::IntegerDistributedRandomSource>();
    herald::datatype::RandomnessGenerator gen(std::move(rnd));
    auto randomV4 = herald::datatype::UUID::random(gen);
    std::string str = randomV4.string();
    INFO("UUID v4 random value: " << str);
    REQUIRE(str != std::string("00000000-0000-4000-8000-000000000000")); // v4 variant 1
  }
}
