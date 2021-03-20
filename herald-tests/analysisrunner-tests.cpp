//  Copyright 2021 Herald project contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

#include <utility>

using namespace herald::analysis::sampling;
using namespace herald::datatype;

template <std::size_t Sz>
struct DummyRSSISource {
  using value_type = Sample<RSSI>; // allows AnalysisRunner to introspect this class at compile time

  DummyRSSISource(const std::size_t srcDeviceKey, SampleList<Sample<RSSI>,Sz>&& data) : key(srcDeviceKey), data(std::move(data)) {};
  ~DummyRSSISource() = default;

  template <typename RunnerT>
  void run(int timeTo, RunnerT& runner) {
    // push through data at default rate
    for (auto& v: data) {
      // devList.push(v.taken,v.value); // copy data over (It's unusual taking a SampleList and sending to a SampleList)
      if (v.taken.secondsSinceUnixEpoch() <= timeTo) {
        runner.template newSample<RSSI>(key,v);
      }
    }
    runner.run(Date(timeTo));
  }

private:
  std::size_t key;
  SampleList<Sample<RSSI>,Sz> data;
};

struct DummyDistanceDelegate /* : herald::analysis::AnalysisDelegate */ {
  using value_type = Distance;

  DummyDistanceDelegate() : lastSampledID(0), distances() {};
  DummyDistanceDelegate(const DummyDistanceDelegate&) = delete; // copy ctor deleted
  DummyDistanceDelegate(DummyDistanceDelegate&& other) noexcept : lastSampledID(other.lastSampledID), distances(std::move(other.distances)) {} // move ctor
  ~DummyDistanceDelegate() {};

  DummyDistanceDelegate& operator=(DummyDistanceDelegate&& other) noexcept {
    lastSampledID = other.lastSampledID;
    std::swap(distances,other.distances);
    return *this;
  }

  // specific override of template
  void newSample(SampledID sampled, Sample<Distance> sample) {
    lastSampledID = sampled;
    distances.push(sample);
  }

  void reset() {
    distances.clear();
    lastSampledID = 0;
  }

  // Test only methods
  SampledID lastSampled() {
    return lastSampledID;
  }

  const SampleList<Sample<Distance>,25>& samples() {
    return distances;
  }

private:
  SampledID lastSampledID;
  SampleList<Sample<Distance>,25> distances;
};


TEST_CASE("variantset-basic", "[variantset][basic]") {
  SECTION("variantset-basic") {
    herald::analysis::VariantSet<int,double> vs;
    REQUIRE(vs.size() == 2);
    int intValue = vs.get<int>();
    REQUIRE(intValue == 0);
    double dblValue = vs.get<double>();
    REQUIRE(dblValue == 0.0);
  }
}

TEST_CASE("variantset-lists", "[variantset][lists]") {
  SECTION("variantset-lists") {
    herald::analysis::VariantSet<SampleList<Sample<int>,15>,SampleList<Sample<double>,15>> vs;
    REQUIRE(vs.size() == 2);
    SampleList<Sample<int>,15>& intValue = vs.get<SampleList<Sample<int>,15>>();
    REQUIRE(intValue.size() == 0);
    SampleList<Sample<double>,15>& dblValue = vs.get<SampleList<Sample<double>,15>>();
    REQUIRE(dblValue.size() == 0);

    intValue.push(0,12);
    dblValue.push(10,14.3);
    dblValue.push(20,15.3);

    SampleList<Sample<int>,15>& intValue2 = vs.get<SampleList<Sample<int>,15>>();
    REQUIRE(intValue2.size() == 1);
    SampleList<Sample<double>,15>& dblValue2 = vs.get<SampleList<Sample<double>,15>>();
    REQUIRE(dblValue2.size() == 2);

  }
}

TEST_CASE("variantset-listmanager", "[variantset][listmanager]") {
  SECTION("variantset-listmanager") {
    using herald::analysis::ListManager;
    herald::analysis::VariantSet<ListManager<int,15>,ListManager<double,15>> vs;
    REQUIRE(vs.size() == 2);
    ListManager<int,15>& intValue = vs.template get<ListManager<int,15>>();
    REQUIRE(intValue.size() == 0);
    ListManager<double,15>& dblValue = vs.template get<ListManager<double,15>>();
    REQUIRE(dblValue.size() == 0);
    
    SampleList<Sample<int>,15>& intList = intValue.list(1234); // list for entity being sampled with SampledId=1234
    REQUIRE(intList.size() == 0);
    SampleList<Sample<double>,15>& dblList = dblValue.list(5678); // list for entity being sampled with SampledId=1234
    REQUIRE(dblList.size() == 0);

    intList.push(Sample(0,12));
    dblList.push(Sample(10,14.3));
    dblList.push(Sample(20,15.3));

    ListManager<int,15>& intValue2 = vs.template get<ListManager<int,15>>();
    REQUIRE(intValue2.size() == 1);
    ListManager<double,15>& dblValue2 = vs.template get<ListManager<double,15>>();
    REQUIRE(dblValue2.size() == 1);
    
    SampleList<Sample<int>,15>& intList2 = intValue2.list(1234); // list for entity being sampled with SampledId=1234
    REQUIRE(intList2.size() == 1);
    SampleList<Sample<double>,15>& dblList2 = dblValue2.list(5678); // list for entity being sampled with SampledId=1234
    REQUIRE(dblList2.size() == 2);

  }
}

/// [Who]   As a DCT app developer
/// [What]  I want to link my live application data to an analysis runner easily
/// [Value] So I don't have to write plumbing code for Herald itself
/// 
/// [Who]   As a DCT app developer
/// [What]  I want to periodically run analysis aggregates automatically
/// [Value] So I don't miss any information, and have accurate, regular, samples
TEST_CASE("analysisrunner-basic", "[analysisrunner][basic]") {
  SECTION("analysisrunner-basic") {
    SampleList<Sample<RSSI>,25> srcData;
    srcData.push(10,-55);
    srcData.push(20,-55);
    srcData.push(30,-55);
    srcData.push(40,-55);
    srcData.push(50,-55);
    srcData.push(60,-55);
    srcData.push(70,-55);
    srcData.push(80,-55);
    srcData.push(90,-55);
    srcData.push(100,-55);
    DummyRSSISource src(1234,std::move(srcData));

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(30, -50, -24);

    DummyDistanceDelegate myDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<DummyDistanceDelegate>,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
      RSSI,Distance
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    // run at different times and ensure that it only actually runs three times (sample size == 3)
    src.run(20,runner);
    src.run(40,runner); // Runs here, because we have data for 10,20,>>30<<,40 <- next run time based on this 'latest' data time
    src.run(60,runner);
    src.run(80,runner); // Runs here because we have extra data for 50,60,>>70<<,80 <- next run time based on this 'latest' data time
    src.run(95,runner);

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 1234);

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 2); // didn't reach 4x30 seconds, so no tenth sample, and didn't run at 60 because previous run was at time 40
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() == 40);
    REQUIRE(samples[0].value != 0.0);
    REQUIRE(samples[1].taken.secondsSinceUnixEpoch() == 80);
    REQUIRE(samples[1].value != 0.0);
  }
}