//  Copyright 2021 Herald project contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include "herald/herald.h"

#include <utility>
#include <iostream>

using namespace herald;
using namespace herald::analysis::sampling;
using namespace herald::datatype;

struct DummyDistanceDelegate {
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

/// [Who]   As a DCT app developer
/// [What]  I want to link my live application data to an analysis runner easily
/// [Value] So I don't have to write plumbing code for Herald itself
/// 
/// [Who]   As a DCT app developer
/// [What]  I want to periodically run analysis aggregates automatically
/// [Value] So I don't miss any information, and have accurate, regular, samples
TEST_CASE("analysissensor-rssi-basic", "[analysissensor][rssi][basic]") {
  SECTION("analysissensor-rssi-basic") {
    Proximity p1{.unit = ProximityMeasurementUnit::RSSI, .value = -55};
    Proximity p2{.unit = ProximityMeasurementUnit::RSSI, .value = -56};
    Proximity p3{.unit = ProximityMeasurementUnit::RSSI, .value = -57};
    Proximity p4{.unit = ProximityMeasurementUnit::RSSI, .value = -58};

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(0, -50, -24); // 0 = run every time run() is called

    DummyDistanceDelegate myDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<DummyDistanceDelegate>,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
      RSSI,Distance
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    herald::analysis::SensorDelegateRSSISource<decltype(runner)> src(runner);
    PayloadData payload(std::byte(5),4);
    TargetIdentifier id(Data(std::byte(3),16));
    src.sensor(SensorType::BLE, p1, id, payload);
    src.sensor(SensorType::BLE, p2, id, payload);
    runner.run(Date()); // In an app, use a Coordinator task
    src.sensor(SensorType::BLE, p3, id, payload);
    src.sensor(SensorType::BLE, p4, id, payload);
    runner.run(Date()); // In an app, use a Coordinator task

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() != 0);

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 1); // Only 1 because time will run in 'real time' as its a sensor source (dynamic date)
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() != 0);
    REQUIRE(samples[0].value != 0.0);

    // Let's see the total memory in use...
    std::cout << "AnalysisRunner::RAM = " << sizeof(runner) << std::endl;
    std::cout << "SensorDelegateRSSISource::RAM = " << sizeof(src) << std::endl;
  }
}



TEST_CASE("analysissensor-output", "[sensorlogger][analysissensor][output]") {
  SECTION("analysissensor-output") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    //herald::data::SensorLogger logger(ctx,"testout","analysissensor");
    herald::analysis::LoggingAnalysisDelegate<CT,Distance> lad(ctx); // The subject of this test
    std::cout << "LoggingAnalysisDelegate::RAM = " << sizeof(lad) << std::endl;

    
    Proximity p1{.unit = ProximityMeasurementUnit::RSSI, .value = -55};
    Proximity p2{.unit = ProximityMeasurementUnit::RSSI, .value = -56};
    Proximity p3{.unit = ProximityMeasurementUnit::RSSI, .value = -57};
    Proximity p4{.unit = ProximityMeasurementUnit::RSSI, .value = -58};

    herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(0, -50, -24); // 0 = run every time run() is called

    DummyDistanceDelegate myDelegate;
    herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate), std::move(lad)); // NOTE: myDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<DummyDistanceDelegate,herald::analysis::LoggingAnalysisDelegate<CT,Distance>>,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
      RSSI,Distance
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

    herald::analysis::SensorDelegateRSSISource<decltype(runner)> src(runner);
    PayloadData payload(std::byte(5),4);
    TargetIdentifier id(Data(std::byte(3),16));
    src.sensor(SensorType::BLE, p1, id, payload);
    src.sensor(SensorType::BLE, p2, id, payload);
    runner.run(Date()); // In an app, use a Coordinator task
    src.sensor(SensorType::BLE, p3, id, payload);
    src.sensor(SensorType::BLE, p4, id, payload);
    runner.run(Date()); // In an app, use a Coordinator task

    auto& delegateRef = adm.get<DummyDistanceDelegate>();
    REQUIRE(delegateRef.lastSampled() != 0);

    auto& samples = delegateRef.samples();
    REQUIRE(samples.size() == 1); // Only 1 because time will run in 'real time' as its a sensor source (dynamic date)
    REQUIRE(samples[0].taken.secondsSinceUnixEpoch() != 0);
    REQUIRE(samples[0].value != 0.0);

    auto lastMsg = dls.value;
    REQUIRE(lastMsg != ""); // must have logged something...
    std::cout << "Last log message: " << lastMsg << std::endl;

    // Let's see the total memory in use...
    std::cout << "AnalysisRunner::RAM = " << sizeof(runner) << std::endl;
    std::cout << "SensorDelegateRSSISource::RAM = " << sizeof(src) << std::endl;


  }
}