//  Copyright 2021 Herald project contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

#include <utility>
#include <iostream>

using namespace herald::analysis::sampling;
using namespace herald::datatype;

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


struct DummyBrightnessDelegate {
  using value_type = RunningMean<Luminosity>;

  DummyBrightnessDelegate() : lastSampledID(0), brightness() {};
  DummyBrightnessDelegate(const DummyBrightnessDelegate&) = delete; // copy ctor deleted
  DummyBrightnessDelegate(DummyBrightnessDelegate&& other) noexcept : lastSampledID(other.lastSampledID), brightness(std::move(other.brightness)) {} // move ctor
  ~DummyBrightnessDelegate() {};

  DummyBrightnessDelegate& operator=(DummyBrightnessDelegate&& other) noexcept {
    lastSampledID = other.lastSampledID;
    std::swap(brightness,other.brightness);
    return *this;
  }

  // specific override of template
  void newSample(SampledID sampled, Sample<RunningMean<Luminosity>> sample) {
    lastSampledID = sampled;
    brightness.push(sample);
  }

  void reset() {
    brightness.clear();
    lastSampledID = 0;
  }

  // Test only methods
  SampledID lastSampled() {
    return lastSampledID;
  }

  const SampleList<Sample<RunningMean<Luminosity>>,25>& samples() {
    return brightness;
  }

private:
  SampledID lastSampledID;
  SampleList<Sample<RunningMean<Luminosity>>,25> brightness;
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

/// Null test case with zero data, no failures, correct summary output
TEST_CASE("analysisrunner-nodata", "[analysisrunner][nodata]") {
  SECTION("analysisrunner-nodata") {
    SampleList<Sample<RSSI>,25> srcData;
    DummySampleSource src(1234,std::move(srcData));

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(30, -50, -24);

    DummyDistanceDelegate myDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<DummyDistanceDelegate>,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
      RSSI,Distance
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    src.run(140,runner);
    REQUIRE(src.getLastRunAdded() == 0); // No data, no run

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 0); // not ran, so 0

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 0); // 0 as there is no source data
  }
}

/// Single data item use case with 1 data item, no failures, correct summary output
TEST_CASE("analysisrunner-singledataitem", "[analysisrunner][singledataitem]") {
  SECTION("analysisrunner-singledataitem") {
    SampleList<Sample<RSSI>,25> srcData;
    srcData.push(50,-55);
    DummySampleSource src(1234,std::move(srcData));

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(30, -50, -24);

    DummyDistanceDelegate myDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<DummyDistanceDelegate>,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
      RSSI,Distance
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    src.run(140,runner);
    REQUIRE(src.getLastRunAdded() == 1); // Single data item

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 1234); // ran once, past 50, for SampleID=1234

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 1); // 1 as single data item
  }
}


/// Single data item use case with 1 data item, no failures, correct summary output
TEST_CASE("analysisrunner-singledataitem-twoanalyses", "[analysisrunner][singledataitem][twoanalyses][multivariate]") {
  SECTION("analysisrunner-singledataitem-twoanalyses") {
    SampleList<Sample<RSSI>,25> srcData;
    srcData.push(50,-55);
    DummySampleSource src(1234,std::move(srcData));
    
    SampleList<Sample<Luminosity>,15> srcLightData;
    srcLightData.push(40,12);
    srcLightData.push(50,12);
    srcLightData.push(60,12);
    DummySampleSource srcLight(5678,std::move(srcLightData));

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(30, -50, -24);
    herald::analysis::algorithms::RunningMeanAnalyser<herald::datatype::Luminosity,2> meanLight;

    DummyDistanceDelegate myDelegate;
    DummyBrightnessDelegate myBrightnessDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate),std::move(myBrightnessDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser), std::move(meanLight)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<
        DummyDistanceDelegate,
        DummyBrightnessDelegate
      >,
      herald::analysis::AnalysisProviderManager<
        herald::analysis::algorithms::distance::FowlerBasicAnalyser,
        herald::analysis::algorithms::RunningMeanAnalyser<herald::datatype::Luminosity,2>
      >,
      RSSI,Distance,Luminosity,RunningMean<Luminosity>
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    src.run(140,runner);
    srcLight.run(160,runner);
    REQUIRE(src.getLastRunAdded() == 1); // Single data item
    REQUIRE(srcLight.getLastRunAdded() == 3); // Three data items

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 1234); // ran once, past 50, for SampleID=1234

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 1); // 1 as single data item (for THIS delegate)
    

    auto& delegateBRef = adm.get<DummyBrightnessDelegate>();
    REQUIRE(delegateBRef.lastSampled() == 5678); // ran once, past 50, for SampleID=1234

    auto& samplesB = delegateBRef.samples();
    REQUIRE(samplesB.size() == 1); // 1 as only one mean generated (one run) for this variable
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
    DummySampleSource src(1234,std::move(srcData));

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
    REQUIRE(src.getLastRunAdded() == 2); // 10, 20 - THIS IS ZERO BUT MUST BE 2!!!
    src.run(40,runner); // Runs here, because we have data for 10,20,>>30<<,40 <- next run time based on this 'latest' data time
    REQUIRE(src.getLastRunAdded() == 2); // 30, 40
    src.run(60,runner);
    REQUIRE(src.getLastRunAdded() == 2); // 50, 60
    src.run(80,runner); // Runs here because we have extra data for 50,60,>>70<<,80 <- next run time based on this 'latest' data time
    REQUIRE(src.getLastRunAdded() == 2); // 70, 80
    src.run(95,runner);
    REQUIRE(src.getLastRunAdded() == 1); // 90

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 1234);

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 2); // didn't reach 4x30 seconds, so no tenth sample, and didn't run at 60 because previous run was at time 40
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() == 40);
    REQUIRE(samples[0].value != 0.0);
    REQUIRE(samples[1].taken.secondsSinceUnixEpoch() == 80);
    REQUIRE(samples[1].value != 0.0);

    // Let's see the total memory in use...
    std::cout << "AnalysisRunner::RAM = " << sizeof(runner) << std::endl;
  }
}


/// [Who]   As a DCT app developer
/// [What]  I want to link my live application data to an analysis runner easily
/// [Value] So I don't have to write plumbing code for Herald itself
/// 
/// [Who]   As a DCT app developer
/// [What]  I want to periodically run analysis aggregates automatically
/// [Value] So I don't miss any information, and have accurate, regular, samples
TEST_CASE("analysisrunner-nonewdata", "[analysisrunner][nonewdata]") {
  SECTION("analysisrunner-nonewdata") {
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
    DummySampleSource src(1234,std::move(srcData));

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
    REQUIRE(src.getLastRunAdded() == 2); // 10, 20
    src.run(40,runner); // Runs here, because we have data for 10,20,>>30<<,40 <- next run time based on this 'latest' data time
    REQUIRE(src.getLastRunAdded() == 2); // 30, 40
    src.run(60,runner);
    REQUIRE(src.getLastRunAdded() == 2); // 50, 60
    src.run(80,runner); // Runs here because we have extra data for 50,60,>>70<<,80 <- next run time based on this 'latest' data time
    REQUIRE(src.getLastRunAdded() == 2); // 70, 80
    src.run(95,runner);
    REQUIRE(src.getLastRunAdded() == 1); // 90

    // Now ensure that running runner past end of data does not cause result to change
    src.run(115,runner); // Run here because we have past 80 + 30
    REQUIRE(src.getLastRunAdded() == 1); // 100
    src.run(150,runner); // Should not run here as there's no new data (even though we're at > 115 + 30)
    REQUIRE(src.getLastRunAdded() == 0); // No new data

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() == 1234);

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 3); // 150 should not result in a run after 145 (min time delay)
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() == 40);
    REQUIRE(samples[0].value != 0.0);
    REQUIRE(samples[1].taken.secondsSinceUnixEpoch() == 80);
    REQUIRE(samples[1].value != 0.0);
    REQUIRE(samples[2].taken.secondsSinceUnixEpoch() == 100); // Last data was at 100, not 115, so it takes that time
    REQUIRE(samples[2].value != 0.0);
    // REQUIRE(samples[3].taken.secondsSinceUnixEpoch() == 150);
    // REQUIRE(samples[3].value != 0.0);
  }
}


