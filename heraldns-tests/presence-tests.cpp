/*
See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  Adam Fowler licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/
#include "catch.hpp"

#include <string>

#include "heraldns/heraldns.h"

TEST_CASE("presencemanager","[presence][basic][presencemanager]") {

  SECTION("presencemanager-basic") {
    heraldns::datatype::PresenceManager pm(64);

    REQUIRE(pm.size() == 64);
    REQUIRE_NOTHROW(pm.get(0));
    REQUIRE_NOTHROW(pm.get(63));
    // REQUIRE_THROWS(pm.get(64));
  }

}

TEST_CASE("presence-ctor","[presence][basic][ctor][presence]") {
  SECTION("presence-ctor") {
    heraldns::datatype::Presence p(5);

    REQUIRE(p.id() == 5);
    REQUIRE(p.risk() == 0);
    REQUIRE(p.newRisk() == 0);
    REQUIRE(p.transmittedRisk() == 0);
    REQUIRE(p.newTransmittedRisk() == 0);
    REQUIRE(p.flightiness() == 0);
    REQUIRE(p.state() == heraldns::datatype::State::Well);
    REQUIRE(p.hasEverBeenIll() == false);
    REQUIRE(p.lastFellIll() == 0);
    REQUIRE(p.lastRecovered() == 0);
    REQUIRE(p.highestRiskScore() == 0);
    REQUIRE(p.transmissionModelScore() == 0);
    REQUIRE(p.newTransmissionModelScore() == 0);
    bool hasValue = (bool)p.position();
    REQUIRE(false == hasValue);
  }
}

// TODO test setters and retrievers without commit

// TODO test commit post-conditions

// TODO test move to and position