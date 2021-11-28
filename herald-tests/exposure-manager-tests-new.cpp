//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

using namespace herald::analysis::sampling;
using namespace herald::datatype;

herald::datatype::UUID dummyAgent = UUID::unknown();

struct DummyExposureStore {

};

struct DummyExposureCallbackHandler {
  void exposureLevelChanged(
    const herald::datatype::ExposureMetadata& meta,
    const herald::datatype::Exposure& exposure) noexcept {
    called = true;
    ++timesCalled;
    agent = meta.agentId;
    currentExposureValue = exposure.value;
  }

  herald::datatype::UUID agent = dummyAgent;
  double currentExposureValue = 0;
  bool called = false;
  std::size_t timesCalled = 0;
};

TEST_CASE("exposure-empty", "[exposure][empty]") {
  SECTION("exposure-empty") {
    // Create exposure manager with no diseases(agents)
    DummyExposureCallbackHandler dh;
    DummyExposureStore des;
    herald::exposure::ExposureManager<DummyExposureCallbackHandler,8, DummyExposureStore> em(dh,des);

    // check static max size
    REQUIRE(herald::exposure::ExposureManager<DummyExposureCallbackHandler,8, DummyExposureStore>::max_size == 8);
    // Check configuration is correct
    REQUIRE(em.sourceCount() == 0);
    // Ensure it is not running by default
    REQUIRE(!em.isRunning());
  }
}


TEST_CASE("exposure-callback-handler", "[exposure][callback][handler") {
  SECTION("exposure-callback-handler") {
    // Create EM so we can reference its delegate
    // Create exposure manager
    DummyExposureCallbackHandler dh;
    DummyExposureStore des;
    herald::exposure::ExposureManager<DummyExposureCallbackHandler,8, DummyExposureStore> em(dh,des);

    // Create underlying AnalysisRunner first
    SampleList<Sample<RSSI>,25> srcData;
    srcData.push(0,-55);
    srcData.push(30,-55);
    srcData.push(60,-55); // one minute (45 rssi-minutes)
    srcData.push(90,-55);
    srcData.push(120,-55); // 90
    srcData.push(150,-55); // 112.5 - OVER THRESHOLD
    srcData.push(180,-55);
    srcData.push(210,-55);
    srcData.push(240,-55);
    srcData.push(270,-55);
    srcData.push(300,-55);
    DummyRSSISource src(1234,std::move(srcData));

    herald::analysis::algorithms::RSSIMinutesAnalyser riskAnalyser;

    auto analysisDelegate = em.template analysisDelegate<herald::datatype::RSSIMinute>(); // full object (not just reference)

    // Note: The delegate manager supports multiple analysis delegates, so you can have one per risk model type (set at compile time)
    herald::analysis::AnalysisDelegateManager adm(std::move(analysisDelegate)); // NOTE: analysisDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(riskAnalyser)); // NOTE: riskAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<
        herald::exposure::ExposureManagerDelegate<
          herald::datatype::RSSIMinute,
          herald::exposure::ExposureManager<DummyExposureCallbackHandler,8, DummyExposureStore>
        >
      >,
      herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::RSSIMinutesAnalyser>,
      RSSI,RSSIMinute
    > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<RSSIMinute>)



    // Some level constants for convenience. These are app-level and variable, and thus not hardcoded as code constants and compiled in
    constexpr std::size_t noAction = 255;
    constexpr std::size_t selfIsolate = 254;


    // Add a single disease(agent)
    herald::datatype::UUID proxInstanceId = 
      herald::datatype::UUID::fromString("99999999-1111-4011-8011-111111111111");
    bool addSuccess1 = em.addSource<RSSIMinute>(
      herald::datatype::agent::humanProximity, 
      sensorClass::bluetoothProximityHerald, proxInstanceId);
    // Note: We've linked the above to a particular Model Type (RSSIMinute), and by default are using instanceID derived from the SampledId
    REQUIRE(addSuccess1);
    REQUIRE(em.sourceCount() == 1);

    // Now run the values through
    em.enableRunning(); // required, else no changes will be recorded
    src.run(301, runner);
    // Now fire off any exposure changes
    bool result = em.notifyOfChanges();

    // Now confirm callback values
    REQUIRE(result); // A notification occured
    INFO("Agent from callback: " << dh.agent.string() << ", but expected: " << herald::datatype::agent::humanProximity.string());
    REQUIRE(dh.agent == herald::datatype::agent::humanProximity);
    REQUIRE(dh.called);
    REQUIRE(1 == dh.called);
    REQUIRE(dh.currentExposureValue == (45 * 5)); // -55 (so 45) RSSI for 5 minutes (RSSIMinutes) (5 minutes elapsed)
  }
}

// [Who]   As an exposure calculator
// [What]  I need to sum exposure of particular types by a particular time period length in each given day
// [Value] In order to summarise exposure correctly for that agent and source

// [Who]   
// [What]  
// [Value] 

// [Who]   
// [What]  
// [Value] 

// [Who]   
// [What]  
// [Value] 



//     // Link an rssi-minutes aggregation risk routine to this disease
//     bool setEM1 = em.template setAgentExposureModel<herald::analysis::algorithms::RSSIMinutesAnalyser>(adamitis);
//     REQUIRE(setEM1);
//     // Add a no action threshold, and a self-isolate threshold
//     bool addEL1 = em.addExposureLevel(adamitis,0,100,noAction); // Using a level ID of 255 for now (no reason) -> no-action
//     REQUIRE(addEL1);
//     REQUIRE(em.getExposureLevelCount(adamitis) == 1);
//     bool addEL2 = em.addExposureLevel(adamitis,100,65535,selfIsolate); // self-isolate level
//     REQUIRE(addEL2);
//     REQUIRE(em.getExposureLevelCount(adamitis) == 2);

//     // Initialise risk states
//     bool setED1 = em.setExposureDefaults(adamitis,1,noAction); // not every risk starts at 0
//     REQUIRE(setED1);
//     auto ed1 = em.getExposureDefaultLevelId(adamitis);
//     REQUIRE(ed1 == noAction);
//     auto iel1 = em.getExposureInitialValue(adamitis);
//     REQUIRE(iel1 == 1.0);
//     REQUIRE(em.getExposureLevelCount(adamitis) == 2);

//     // Ensure the initial state is no action
//     auto cev1 = em.getExposureCurrentValue(adamitis);
//     REQUIRE(cev1 == 1.0);
//     auto cel1 = em.getExposureCurrentLevelId(adamitis);
//     REQUIRE(noAction == cel1);

//     // Start running
//     em.setRunning(true);

//     // Ensure the initial state is still no action
//     auto cev1s = em.getExposureCurrentValue(adamitis);
//     REQUIRE(cev1s == 1.0);
//     auto cel1s = em.getExposureCurrentLevelId(adamitis);
//     REQUIRE(noAction == cel1s);
//     // Add some risk (below threshold) - via analysis runner
//     for (int time = 30;time <=120;time += 30) {
//       // ensure we're calculating at a realistic runtime rate (~5s in real life)
//       src.run(time,runner);
//     }
//     // Ensure the initial state is still no action
//     cel1s = em.getExposureCurrentLevelId(adamitis);
//     REQUIRE(noAction == cel1s);
//     // Ensure the risk value is as expected (90 + 1 RSSI Minutes)
//     cev1 = em.getExposureCurrentValue(adamitis);
//     REQUIRE(cev1 == 91.0);
//     // Add some risk (above threshold)
//     src.run(150,runner);

    
//     // Ensure our risk state changed callback has not yet been called
//     REQUIRE(!dh.called);

//     // Now re-evaluate levels
//     em.evaluateLevels();
//     // Ensure our risk state changed callback has been called
//     REQUIRE(dh.called);
//     REQUIRE(dh.agent.id == adamitis.id);
//     REQUIRE(dh.levelId == selfIsolate);
//     REQUIRE(dh.currentExposureValue == 113.5);

//     // Ensure state is now self-isolate
//     cel1s = em.getExposureCurrentLevelId(adamitis);
//     REQUIRE(selfIsolate == cel1s);
//     // Ensure drop back time is set appropriately (112.5 + 1 RSSI Minutes)
//     cev1 = em.getExposureCurrentValue(adamitis);
//     REQUIRE(cev1 == 113.5);

//     // Re-evaluate state, ensuring it is still self-isolate
//     dh.called = false; // reset call flag
//     em.evaluateLevels();
//     cel1s = em.getExposureCurrentLevelId(adamitis);
//     REQUIRE(selfIsolate == cel1s);
//     cev1 = em.getExposureCurrentValue(adamitis);
//     REQUIRE(cev1 == 113.5);
//     // Ensure our risk state changed callback has not been called again
//     REQUIRE(!dh.called);

//     // Now remove agent and associated info (including levels)
//     em.removeAgent(adamitis);
//     REQUIRE(em.agentCount() == 0);
//     REQUIRE(em.getExposureLevelCount(adamitis) == 0);
//   }
// }

