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
#include "tests.h"

#include <filesystem>
#include <string>

#include "heraldns/heraldns.h"

namespace fs = std::filesystem;

TEST_CASE("cell","[cell][basic][datatypes]") {

  SECTION("cell-basic") {
    std::shared_ptr<heraldns::datatype::Cell> cell = std::make_shared<heraldns::datatype::Cell>(5,10);
    
    // check basic metrics
    REQUIRE(cell->x() == 5);
    REQUIRE(cell->y() == 10);
    REQUIRE(cell->present().size() == 0);

  }

}

TEST_CASE("grid","[grid][basic][datatypes]") {

  SECTION("grid-basic") {
    std::shared_ptr<heraldns::datatype::Grid> grid = std::make_shared<heraldns::datatype::Grid>(100, 90, 0.5);
    
    // check basic metrics
    REQUIRE(grid->width() == 100);
    REQUIRE(grid->height() == 90);
    REQUIRE(grid->separation() == 0.5);

    // check grid allocation and positional fetch
    auto cNorth = grid->cell(5,10);
    REQUIRE(cNorth->x() == 5);
    REQUIRE(cNorth->y() == 10);
    auto cSouth = grid->cell(5,30);
    REQUIRE(cSouth->x() == 5);
    REQUIRE(cSouth->y() == 30);
    auto cWest = grid->cell(0,20);
    REQUIRE(cWest->x() == 0);
    REQUIRE(cWest->y() == 20);
    auto cEast = grid->cell(12,20);
    REQUIRE(cEast->x() == 12);
    REQUIRE(cEast->y() == 20);
    auto cCentre = grid->cell(5,20);
    REQUIRE(cCentre->x() == 5);
    REQUIRE(cCentre->y() == 20);
    
    // check distance function
    REQUIRE(grid->distance(cNorth,cSouth) == 10); // 20 cells * 0.5 cell separation distance (metres)
    REQUIRE(grid->distance(cSouth,cNorth) == 10);
    REQUIRE(grid->distance(cWest,cEast) == 6);
    REQUIRE(grid->distance(cEast,cWest) == 6);

  }

}