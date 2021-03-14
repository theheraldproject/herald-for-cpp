//  Copyright 2021 Herald project contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

using namespace herald::analysis::sampling;
using namespace herald::datatype;

template <std::size_t Sz>
struct DummyRSSISource {
  using value_type = Sample<RSSI>; // allows AnalysisRunner to introspect this class at compile time

  DummyRSSISource(const std::size_t srcDeviceKey, SampleList<Sample<RSSI>,Sz>&& data) : key(srcDeviceKey), data(std::move(data)) {};
  ~DummyRSSISource() = default;

  template <typename RunnerT>
  void run(int timeTo, RunnerT& runner) {
    // get reference to target list
    // auto& devList = runner.list<RSSI>(key);
    // push through data at default rate
    for (auto& v: data) {
      // devList.push(v.taken,v.value); // copy data over (It's unusual taking a SampleList and sending to a SampleList)
      if (v.taken.secondsSinceUnixEpoch() <= timeTo) {
        runner.newSample<20>(key,v);
      }
    }
    runner.run(timeTo);
  }

private:
  std::size_t key;
  SampleList<Sample<RSSI>,Sz> data;
};

struct DummyDistanceDelegate {
  DummyDistanceDelegate() = default;
  ~DummyDistanceDelegate() = default;

  // Detected methods by AnalysisRunner
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

  const SampleList<Sample<Distance>,20>& samples() {
    return distances;
  }

private:
  SampledID lastSampledID;
  SampleList<Sample<Distance>,20> distances;
};

/// [Who]   As a DCT app developer
/// [What]  I want to link my live application data to an analysis runner easily
/// [Value] So I don't have to write plumbing code for Herald itself
/// 
/// [Who]   As a DCT app developer
/// [What]  I want to periodically run analysis aggregates automatically
/// [Value] So I don't miss any information, and have accurate, regular, samples
TEST_CASE("analysisrunner-basic", "[analysisrunner][basic]") {
  SECTION("analysisrunner-basic") {
    SampleList<Sample<RSSI>,10> srcData{
      Sample<RSSI>{10,-55},Sample<RSSI>{20,-55},Sample<RSSI>{30,-55},Sample<RSSI>{40,-55},Sample<RSSI>{50,-55},
      Sample<RSSI>{60,-55},Sample<RSSI>{70,-55},Sample<RSSI>{80,-55},Sample<RSSI>{90,-55},Sample<RSSI>{100,-55}
    };
    DummyRSSISource src(1234,std::move(srcData));

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(30, -50, -24);

    herald::analysis::AnalysisRunner<RSSI,Distance> runner; // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    DummyDistanceDelegate myDelegate;
    runner.add(distanceAnalyser); // so it picks up new data items
    runner.add(myDelegate); // so we receive the relevant output

    // run at different times and ensure that it only actually runs three times (sample size == 3)
    src.run(20,runner);
    src.run(40,runner);
    src.run(60,runner);
    src.run(80,runner);
    src.run(95,runner);

    auto& samples = myDelegate.samples();
    REQUIRE(samples.size() == 3); // didn't reach 4x30 seconds, so no tenth sample
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() == 30);
    REQUIRE(samples[0].value != 0.0);
    REQUIRE(samples[1].taken.secondsSinceUnixEpoch() == 60);
    REQUIRE(samples[1].value != 0.0);
    REQUIRE(samples[2].taken.secondsSinceUnixEpoch() == 90);
    REQUIRE(samples[2].value != 0.0);
  }
}