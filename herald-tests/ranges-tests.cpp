//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include <iterator>
#include <iostream>

#include "herald/herald.h"

TEST_CASE("ranges-iterator-proxy", "[ranges][iterator][proxy]") {
  SECTION("ranges-iterator-proxy") {
    herald::analysis::views::in_range<int> workingAge(18,65);

    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<int>,5> ages;
    ages.push(10,12);
    ages.push(20,14);
    ages.push(30,19);
    ages.push(40,45);
    ages.push(50,66);
    herald::analysis::views::iterator_proxy proxy(ages);

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

    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<int>,5> ages;
    ages.push(10,12);
    ages.push(20,14);
    ages.push(30,19);
    ages.push(40,45);
    ages.push(50,66);

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
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<int>,5> ages;
    ages.push(10,12);
    ages.push(20,14);
    ages.push(30,19);
    ages.push(40,45);
    ages.push(50,66);

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
    
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<int>,5> ages;
    ages.push(10,12);
    ages.push(20,14);
    ages.push(30,19);
    ages.push(40,45);
    ages.push(50,66);

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
                  | herald::analysis::views::filter(
                      herald::analysis::views::in_range(
                        mode - 2*sd, // NOTE: WE USE THE MODE FOR FILTER, BUT SD FOR BOUNDS - See website for the reasoning
                        mode + 2*sd
                      )
                    )
                  // | herald::analysis::views::to_view() // returns an l-value -> Have to wrap in a view here as we need an end iterator to evaluate in aggregate
                  | aggregate(to_distance); // type actually <herald::analysis::algorithms::distance::FowlerBasic>
    
    auto agg = distance.get<herald::analysis::algorithms::distance::FowlerBasic>();
    auto d = agg.reduce();
    REQUIRE((d > 5.623 && d < 5.624)); // double rounding

    // Now do the same for an in-line temporary aggregate...
    
    auto distance2 = sl 
                   | herald::analysis::views::filter(afterPoint)
                   | herald::analysis::views::filter(valid) 
                   | herald::analysis::views::filter(strong)
                   | herald::analysis::views::filter(
                       herald::analysis::views::in_range(
                         mode - 2*sd, // NOTE: WE USE THE MODE FOR FILTER, BUT SD FOR BOUNDS - See website for the reasoning
                         mode + 2*sd
                       )
                     )
                  //  | herald::analysis::views::to_view() // returns an l-value -> Now have a helper in aggregate so we don't need to use to_view
                   | aggregate(herald::analysis::algorithms::distance::FowlerBasic(-50, -24)); // TRYING WITH A TEMPORARY - CHECKING IT DOES STD::MOVE CORRECTLY
    
    auto agg2 = distance2.get<herald::analysis::algorithms::distance::FowlerBasic>();
    auto d2 = agg2.reduce();
    REQUIRE((d2 > 5.623 && d2 < 5.624)); // double rounding
  }
}

// Risk aggregation example implementation
TEST_CASE("ranges-risk-aggregate", "[ranges][risk][aggregate][no-filter]") {
  SECTION("ranges-risk-aggregate") {
    // First we simulate a list of actual distance samples over time, using a vector of pairs
    std::vector<std::pair<herald::datatype::Date,double>> sourceDistances;
    sourceDistances.emplace_back(1235,5.5);
    sourceDistances.emplace_back(1240,4.7);
    sourceDistances.emplace_back(1245,3.9);
    sourceDistances.emplace_back(1250,3.2);
    sourceDistances.emplace_back(1255,2.2);
    sourceDistances.emplace_back(1260,1.9);
    sourceDistances.emplace_back(1265,1.0);
    sourceDistances.emplace_back(1270,1.3);
    sourceDistances.emplace_back(1275,2.0);
    sourceDistances.emplace_back(1280,2.2);

    // The below would be in your aggregate handling code...
    herald::analysis::sampling::SampleList<herald::analysis::sampling::Sample<double>, 2> distanceList;

    // For n distances we maintain n-1 distance-risks in a list, and continuously add to it
    // (i.e. we don't recalculate risk over all previous time - too much data)
    // Instead we keep a distance-time number for this known 'contact' which lasts up to 15 minutes.
    // (i.e. when the mac address changes in Bluetooth)
    // We would then store that single risk-time number against that single contact ID - much less data!
    double timeScale = 1.0; // default is 1 second
    double distanceScale = 1.0; // default is 1 metre, not scaled
    double minimumDistanceClamp = 1.0; // As per Oxford Risk Model, anything < 1m ...
    double minimumRiskScoreAtClamp = 1.0; // ...equals a risk of 1.0, ...
    // double logScale = 1.0; // ... and falls logarithmically after that
    // NOTE: The above values are pick for testing and may not be epidemiologically accurate!
    herald::analysis::algorithms::risk::RiskAggregationBasic riskScorer(timeScale,distanceScale,minimumDistanceClamp,minimumRiskScoreAtClamp);

    using namespace herald::analysis::aggregates;
    
    // this does nothing other than initialise our riskSlice reference
    auto riskSlice = distanceList
                    // no filters or any other iterator-proxy style class here...
                    //| herald::analysis::views::to_view() // Now we can pipe lists straight in to aggregates and summarise calls
                    | aggregate(riskScorer); // moves riskScorer in to aggregate instance

    // Now generate a sequence of Risk Scores over time
    double interScore = 0.0;
    double firstNonZeroInterScore = 0.0;
    for (auto&[when,distance] : sourceDistances) {
      // A new distance has been calculated!
      distanceList.push(when,distance);
      // Let's see if we have a new risk score!
      riskSlice = distanceList
                // no filters or any other iterator-proxy style class here...
                | riskSlice;
      // Add to our exposure risk for THIS contact
      // Note: We're NOT resetting over time, as the riskScorer will hold our total risk exposure from us.
      //       We could instead extract this slice, store it in a counter, and reset the risk Scorer if
      //       we needed to alter the value somehow or add the risk slices themselves to a new list.
      //       Instead, we only do this for each contact in total (i.e. up to 15 minutes per riskScorer).
      auto& agg = riskSlice.get<herald::analysis::algorithms::risk::RiskAggregationBasic>();
      interScore = agg.reduce();
      if (firstNonZeroInterScore == 0.0 && interScore > 0) {
        firstNonZeroInterScore = interScore;
      }
      std::cout << "RiskAggregationBasic inter score: " << interScore << " address of agg: " << &agg << std::endl;
    }

    // Now we have the total for our 'whole contact duration', not scaled for how far in the past it is
    auto& agg = riskSlice.get<herald::analysis::algorithms::risk::RiskAggregationBasic>();
    double riskScore = agg.reduce();
    std::cout << "RiskAggregationBasic final score: " << riskScore << " address of agg: " << &agg << std::endl;
    REQUIRE(interScore > 0.0); // final inter score should be non zero
    REQUIRE(riskScore > 0.0); // final score should be non zero
    REQUIRE(riskScore > firstNonZeroInterScore); // should be additive over time too
  }
}

// TODO Given a list of risk-distance numbers, and the approximate final time of that contact, calculate
//      a risk score when the risk of infection drops off linearly over 14 days. (like COVID-19)
//      (Ideally we'd have a more robust epidemiological model, but this will suffice for example purposes)
