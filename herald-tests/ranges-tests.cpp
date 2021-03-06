//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include <iterator>

#include "herald/herald.h"

TEST_CASE("ranges-iterator-proxy", "[ranges][iterator][proxy]") {
  SECTION("ranges-iterator-proxy") {
    herald::analysis::views::in_range<int> workingAge(18,65);

    std::vector<int> ages;
    ages.push_back(12);
    ages.push_back(14);
    ages.push_back(19);
    ages.push_back(45);
    ages.push_back(66);
    herald::analysis::views::iterator_proxy<std::vector<int>> proxy(ages);

    REQUIRE(!proxy.ended());
    REQUIRE(*proxy == 12);
    ++proxy;
    REQUIRE(*proxy == 14);
    ++proxy;
    REQUIRE(*proxy == 19);
    ++proxy;
    REQUIRE(*proxy == 45);
    ++proxy;
    REQUIRE(*proxy == 66);
    ++proxy;
    REQUIRE(proxy.ended());
  }
}

TEST_CASE("ranges-filter-typed", "[ranges][typed]") {
  SECTION("ranges-filter-typed") {
    herald::analysis::views::in_range<int> workingAge(18,65);

    std::vector<int> ages;
    ages.push_back(12);
    ages.push_back(14);
    ages.push_back(19);
    ages.push_back(45);
    ages.push_back(66);

    herald::analysis::views::filter<herald::analysis::views::in_range<int>> workingAgeFilter(workingAge);

    auto iter = workingAgeFilter(ages);
    REQUIRE(!iter.ended());
    REQUIRE(*iter == 19);
    ++iter;
    REQUIRE(!iter.ended());
    REQUIRE(*iter == 45);
    ++iter;
    REQUIRE(iter.ended());
    // REQUIRE(thirdOfWorkingAge == std::end(ages));
  }
}

// TODO a version like the above but using generic ranges/views to remove type names

TEST_CASE("ranges-filter-generic", "[ranges][generic]") {
  SECTION("ranges-filter-generic") {
    herald::analysis::views::in_range workingAge(18,65);
    
    std::vector<int> ages;
    ages.push_back(12);
    ages.push_back(14);
    ages.push_back(19);
    ages.push_back(45);
    ages.push_back(66);

    auto workingAges = ages 
                     | herald::analysis::views::filter(workingAge) 
                     | herald::analysis::views::to_view();

    auto iter = workingAges.begin();
    REQUIRE(iter != workingAges.end());
    REQUIRE(*iter == 19);
    ++iter;
    REQUIRE(*iter == 45);

    // TODO make the below work just like any other STL collection
    REQUIRE(workingAges.size() == 2);
    REQUIRE(workingAges[0] == 19);
    REQUIRE(workingAges[1] == 45);
  }
}

TEST_CASE("ranges-filter-multi", "[ranges][filter][multi]") {
  SECTION("ranges-filter-multi") {
    herald::analysis::views::in_range workingAge(18,65);
    herald::analysis::views::greater_than over21(21);
    
    std::vector<int> ages;
    ages.push_back(12);
    ages.push_back(14);
    ages.push_back(19);
    ages.push_back(45);
    ages.push_back(66);

    auto workingAges = ages 
                     | herald::analysis::views::filter(workingAge) 
                     | herald::analysis::views::filter(over21)
                     | herald::analysis::views::to_view();

    auto iter = workingAges.begin();
    REQUIRE(iter != workingAges.end());
    REQUIRE(*iter == 45);
    ++iter;
    REQUIRE(iter == workingAges.end());

    // TODO make the below work just like any other STL collection
    REQUIRE(workingAges.size() == 1);
    REQUIRE(workingAges[0] == 45);
  }
}

// TEST_CASE("analysis-rssi-distance", "[analysis][rssi]") {
//   SECTION("analysis-rssi-distance") {
//     std::array<int,100> rssiList;
//     auto filtered = rssiList 
//                   | filter(in_time(now(),now() - 30))
//                   | filter(in_range(-5,-99));
//     auto summary = filtered | summarise(Mean,SD);
//     auto distance = filtered 
//                   | filter(in_range(mean(summary) - 2*sd(summary), mean(summary) + 2*sd(summary)))
//                   | aggregate(adams_distance_conversion);


//   }
// }