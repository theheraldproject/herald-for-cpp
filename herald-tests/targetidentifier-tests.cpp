//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("datatypes-targetidentifier-ctor-default", "[datatypes][targetidentifier][ctor][default]") {
  SECTION("datatypes-targetidentifier-ctor-default") {
    herald::datatype::TargetIdentifier t1;

    REQUIRE(((std::string)t1).size() == 0);
  }
}

TEST_CASE("datatypes-targetidentifier-ctor-data", "[datatypes][targetidentifier][ctor][data]") {
  SECTION("datatypes-targetidentifier-ctor-data") {
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    herald::datatype::Data out = (herald::datatype::Data)t1;
    REQUIRE(d == out);
  }
}

TEST_CASE("datatypes-targetidentifier-ctor-copy", "[datatypes][targetidentifier][ctor][copy]") {
  SECTION("datatypes-targetidentifier-ctor-copy") {
    herald::datatype::Data d1{std::byte('a'),6};
    herald::datatype::Data d2{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d1);
    herald::datatype::TargetIdentifier t2(d2);

    herald::datatype::Data t1out = (herald::datatype::Data)t1;
    herald::datatype::Data t2out = (herald::datatype::Data)t2;
    REQUIRE(t1 == t2);
    REQUIRE(&t1 != &t2);
    REQUIRE(t1out == t2out);
  }
}

TEST_CASE("datatypes-targetidentifier-comparison", "[datatypes][targetidentifier][comparison]") {
  SECTION("datatypes-targetidentifier-comparison") {
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);
    herald::datatype::TargetIdentifier t2(d);
    
    herald::datatype::Data d3{std::byte('b'),6};
    herald::datatype::TargetIdentifier t3(d3);

    herald::datatype::Data t1out = (herald::datatype::Data)t1;
    herald::datatype::Data t2out = (herald::datatype::Data)t2;
    herald::datatype::Data t3out = (herald::datatype::Data)t3;

    REQUIRE(t1 == t2);
    REQUIRE(!(t1 != t2));
    REQUIRE(t1 != t3);
    REQUIRE(!(t1 == t3));
    REQUIRE(&t1 != &t2);
    REQUIRE(t1out == t2out);
    REQUIRE(t1out != t3out);

    // Test Data equality operator
    REQUIRE(t1 == t1out);
    REQUIRE(t1out == t1);
    REQUIRE(!(t1 != t1out));
    REQUIRE(!(t1out != t1));

    // hash codes
    REQUIRE(t1.hashCode() == t2.hashCode());
    REQUIRE(t1.hashCode() != t3.hashCode());

    // string representation comparison
    REQUIRE(((std::string)t1) == ((std::string)t2));
    REQUIRE(((std::string)t1) != ((std::string)t3));
  }
}