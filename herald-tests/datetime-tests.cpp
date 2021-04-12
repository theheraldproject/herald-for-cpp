//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"


TEST_CASE("datatypes-date-basics", "[datatypes][date][basics]") {
  SECTION("datatypes-date-basics") {
    herald::datatype::Date d(1608483600); // long ctor

    REQUIRE(d.secondsSinceUnixEpoch() == 1608483600);
    REQUIRE(d.iso8601DateTime() == std::string("2020-12-20T17:00:00Z"));
    REQUIRE(((std::string)d) == std::string("2020-12-20T17:00:00Z"));

    herald::datatype::Date d2(d); // copy ctor
    REQUIRE(d2.secondsSinceUnixEpoch() == 1608483600);
    REQUIRE(d2.iso8601DateTime() == std::string("2020-12-20T17:00:00Z"));
    REQUIRE(((std::string)d2) == std::string("2020-12-20T17:00:00Z"));

    // TODO Default constructor producing 'now'
  }
}



TEST_CASE("datatypes-timeinterval-basics", "[datatypes][timeinterval][ctor][basics]") {
  SECTION("datatypes-timeinterval-basics") {
    herald::datatype::TimeInterval ti{1200};

    REQUIRE(ti.millis() == 1'200'000);

    auto t2 = herald::datatype::TimeInterval::never();
    REQUIRE(t2.millis() == LONG_MAX);
    REQUIRE(((std::string)t2) == std::string("never"));

    auto t3 = herald::datatype::TimeInterval::minutes(20);
    REQUIRE(t3.millis() == 20 * 60 * 1000);

    auto t4 = herald::datatype::TimeInterval::seconds(20);
    REQUIRE(t4.millis() == 20 * 1000);

    auto t5 = herald::datatype::TimeInterval::zero();
    REQUIRE(t5.millis() == 0);

    herald::datatype::Date d1{1000};
    herald::datatype::Date d2{1200};
    herald::datatype::TimeInterval t6(d1,d2);

    REQUIRE(t6.millis() == 200 * 1000);
    REQUIRE(((std::string)t6) == std::string("200"));

    REQUIRE(t5 < ti);
    REQUIRE(t5 < t2);
    REQUIRE(t5 < t3);
    REQUIRE(!(t5 > t3));
    REQUIRE(t5 < t4);
  }
}


TEST_CASE("datatypes-timeinterval-date", "[datatypes][timeinterval][ctor][date]") {
  SECTION("datatypes-timeinterval-date") {
    herald::datatype::Date earlier{1200};
    herald::datatype::Date now{1500};

    herald::datatype::TimeInterval difference{earlier,now};
    REQUIRE(difference.seconds() == 300);

    herald::datatype::TimeInterval reverseDifference{now,earlier};
    REQUIRE(reverseDifference.seconds() == -300);

    herald::datatype::Date advanced = earlier + difference;
    REQUIRE(advanced == now);
  }
}


TEST_CASE("datatypes-timeinterval-daterelative", "[datatypes][timeinterval][ctor][daterelative]") {
  SECTION("datatypes-timeinterval-daterelative") {
    herald::datatype::Date now;
    herald::datatype::TimeInterval threeHundred(300);
    herald::datatype::Date earlier = now - threeHundred;

    herald::datatype::TimeInterval difference{earlier,now};
    REQUIRE(difference.seconds() == 300);

    herald::datatype::TimeInterval reverseDifference{now,earlier};
    REQUIRE(reverseDifference.seconds() == -300);

    herald::datatype::Date advanced = earlier + difference;
    REQUIRE(advanced == now);
  }
}