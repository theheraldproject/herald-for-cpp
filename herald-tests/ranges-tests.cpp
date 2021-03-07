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
  }
}

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

    REQUIRE(workingAges.size() == 1);
    REQUIRE(workingAges[0] == 45);
  }
}

TEST_CASE("ranges-iterator-rssisamples", "[ranges][iterator][rssisamples][rssi]") {
  SECTION("ranges-iterator-rssisamples") {
    herald::analysis::views::in_range valid(-99,-10);
    herald::analysis::views::less_than strong(-59);
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,5> sl;
    sl.push(1234,-9);
    sl.push(1244,-60);
    sl.push(1265,-58);
    sl.push(1282,-61);
    sl.push(1294,-100);

    herald::analysis::views::iterator_proxy<herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,5>> proxy(sl);

    REQUIRE(!proxy.ended());
    REQUIRE((*proxy).value == -9);
    ++proxy;
    REQUIRE((*proxy).value == -60);
    ++proxy;
    REQUIRE((*proxy).value == -58);
    ++proxy;
    REQUIRE((*proxy).value == -61);
    ++proxy;
    REQUIRE((*proxy).value == -100);
    ++proxy;
    REQUIRE(proxy.ended());
  }
}

TEST_CASE("ranges-filter-multi-rssisamples", "[ranges][filter][multi][rssisamples][rssi]") {
  SECTION("ranges-filter-multi-rssisamples") {
    herald::analysis::views::in_range valid(-99,-10);
    herald::analysis::views::less_than strong(-59);
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,5> sl;
    sl.push(1234,-9);
    sl.push(1244,-60);
    sl.push(1265,-58);
    sl.push(1282,-61);
    sl.push(1294,-100);

    auto values = sl 
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::filter(strong)
                | herald::analysis::views::to_view();

    auto iter = values.begin();
    REQUIRE(iter != values.end());
    REQUIRE((*iter).value == -60);
    ++iter;
    REQUIRE((*iter).value == -61);
    ++iter;
    REQUIRE(iter == values.end());

    REQUIRE(values.size() == 2);
    auto val0 = values[0].value.intValue();
    auto val1 = values[1].value.intValue();
    REQUIRE(val0 == -60);
    REQUIRE(val1 == -61);
  }
}

TEST_CASE("ranges-filter-multi-summarise", "[ranges][filter][multi][summarise][rssi]") {
  SECTION("ranges-filter-multi-summarise") {
    herald::analysis::views::in_range valid(-99,-10);
    herald::analysis::views::less_than strong(-59);
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,20> sl;
    sl.push(1234,-9);
    sl.push(1244,-60);
    sl.push(1265,-58);
    sl.push(1282,-62);
    sl.push(1282,-68);
    sl.push(1282,-68);
    sl.push(1294,-100);

    using namespace herald::analysis::aggregates;
    auto values = sl 
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::filter(strong)
                | herald::analysis::views::to_view();

    auto summary = values 
                 | summarise<Mean,Mode,Variance>();

    auto mean = summary.get<Mean>();
    auto mode = summary.get<Mode>();
    auto var = summary.get<Variance>();

    REQUIRE(mean == -64.5); // note conversion from RSSI -> int then aggregate -> float
    REQUIRE(mode == -68);
    REQUIRE(var == 17); // Happens to be exact, but you may need take in to account floating point inaccuracy in the tail 
  }
}

TEST_CASE("ranges-filter-multi-since-summarise", "[ranges][filter][multi][since][summarise][rssi]") {
  SECTION("ranges-filter-multi-since-summarise") {
    herald::analysis::views::in_range valid(-99,-10);
    herald::analysis::views::less_than strong(-59);
    herald::analysis::views::since afterPoint(herald::datatype::Date{1245});
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,20> sl;
    sl.push(1234,-9);
    sl.push(1244,-60);
    sl.push(1265,-58);
    sl.push(1282,-62);
    sl.push(1282,-68);
    sl.push(1282,-68);
    sl.push(1294,-100);

    using namespace herald::analysis::aggregates;
    auto values = sl 
                | herald::analysis::views::filter(afterPoint)
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::filter(strong)
                | herald::analysis::views::to_view();

    auto summary = values 
                 | summarise<Mean,Mode,Variance>();

    auto mean = summary.get<Mean>();
    auto mode = summary.get<Mode>();
    auto var = summary.get<Variance>();

    REQUIRE(mean == -66); // note conversion from RSSI -> int then aggregate -> float
    REQUIRE(mode == -68);
    REQUIRE(var == 12); // Happens to be exact, but you may need take in to account floating point inaccuracy in the tail 
  }
}

TEST_CASE("ranges-distance-aggregate", "[ranges][distance][filter][multi][since][summarise][rssi][aggregate]") {
  SECTION("ranges-distance-aggregate") {
    herald::analysis::views::in_range valid(-99,-10);
    herald::analysis::views::less_than strong(-59);
    herald::analysis::views::since afterPoint(herald::datatype::Date{1245});
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<herald::datatype::RSSI>,20> sl;
    sl.push(1234,-9);
    sl.push(1244,-60);
    sl.push(1265,-58);
    sl.push(1282,-62);
    sl.push(1282,-68);
    sl.push(1282,-68);
    sl.push(1294,-100);

    using namespace herald::analysis::aggregates;
    auto values = sl 
                | herald::analysis::views::filter(afterPoint)
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::filter(strong)
                | herald::analysis::views::to_view();

    auto summary = values // is an r-value here
                 | summarise<Mean,Mode,Variance>();

    auto mean = summary.get<Mean>();
    auto mode = summary.get<Mode>();
    auto var = summary.get<Variance>();
    auto sd = std::sqrt(var);

    // See second diagram at https://vmware.github.io/herald/bluetooth/distance
    // i.e. https://vmware.github.io/herald/images/distance-rssi-regression.png
    herald::analysis::algorithms::distance::FowlerBasic to_distance(-50, -24);

    auto distance = sl 
                | herald::analysis::views::filter(afterPoint)
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::filter(strong)
                  //values 
                  | herald::analysis::views::filter(
                      herald::analysis::views::in_range(
                        mean - 2*sd, 
                        mean + 2*sd
                      )
                    )
                  | herald::analysis::views::to_view() // returns an l-value
                  | aggregate<herald::analysis::algorithms::distance::FowlerBasic>(to_distance); // TODO convenience method for this
    
    auto agg = distance.get<herald::analysis::algorithms::distance::FowlerBasic>();
    auto d = agg.reduce();
    REQUIRE((d > 5.623 && d < 5.624)); // double rounding
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