//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

using namespace herald::analysis::sampling;
using namespace herald::datatype;


TEST_CASE("exposure-empty", "[exposure][empty]") {
  SECTION("exposure-empty") {
    // Create exposure manager with no diseases(agents)
    NoOptPassthrough nopt;
    DummyExposureCallbackHandlerNoOpt dh{nopt};
    herald::exposure::FixedMemoryExposureStore<8> des;
    herald::exposure::ExposureManager<DummyExposureCallbackHandlerNoOpt, herald::exposure::FixedMemoryExposureStore<8>> em(dh,des);

    // check static max size
    REQUIRE(herald::exposure::FixedMemoryExposureStore<8>::max_size == 8);
    // Check configuration is correct
    REQUIRE(em.sourceCount() == 0);
    // Ensure it is not running by default
    REQUIRE(!em.isRunning());
  }
}


TEST_CASE("exposure-callback-handler", "[exposure][callback][handler]") {
  SECTION("exposure-callback-handler") {
    // Create EM so we can reference its delegate
    // Create exposure manager
    NoOptPassthrough nopt;
    DummyExposureCallbackHandlerNoOpt dh{nopt};
    herald::exposure::FixedMemoryExposureStore<8> des;
    herald::exposure::ExposureManager<DummyExposureCallbackHandlerNoOpt, herald::exposure::FixedMemoryExposureStore<8>> em(dh,des);
    
    bool setGlobal = em.setGlobalPeriodInterval(0,60); // 60 second windows starting at DateTime==0

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
    DummySampleSource src(1234,std::move(srcData));

    herald::analysis::algorithms::RSSIMinutesAnalyser riskAnalyser{60}; // One RSSIMinute every 60 seconds of input
    // Results in RSSIMinute values of:-
    // 0 (@60 secs) = 45
    // 1 (@120 secs) = 45
    // 2 (@180 secs) = 45
    // 3 (@240 secs) = 45
    // 4 (@300 secs) = 45
    // Resulting in humanProx exposure values of:-
    // 0 (60-60 secs) = 45
    // 1 (60-120 secs) = 45
    // 2 (120-180 secs) = 45
    // 3 (180-240 secs) = 45
    // 4 (240-300 secs) = 45
    // Total = 4.5*45 = 202.5

    auto analysisDelegate = em.template analysisDelegate<herald::datatype::RSSIMinute>(); // full object (not just reference)

    // Note: The delegate manager supports multiple analysis delegates, so you can have one per risk model type (set at compile time)
    herald::analysis::AnalysisDelegateManager adm(std::move(analysisDelegate)); // NOTE: analysisDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(riskAnalyser)); // NOTE: riskAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<
        herald::exposure::ExposureManagerDelegate<
          herald::datatype::RSSIMinute,
          herald::exposure::ExposureManager<DummyExposureCallbackHandlerNoOpt, herald::exposure::FixedMemoryExposureStore<8>>
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
    // WARNING: This callback adds exposure no matter the start/end time period. In your Risk algorithms, scale by ACTUAL exposure start/end times
    REQUIRE(dh.currentExposureValue == (45 * 5)); // -55 (so 45) RSSI for 5 minutes (RSSIMinutes) (5 minutes elapsed, but 0 index on RSSIMinute)
  }
}

// [Who]   As an exposure calculator
// [What]  I need to sum exposure of particular types by a particular time period length in each given day
// [Value] In order to summarise exposure correctly for that agent and source

TEST_CASE("exposure-time-periods", "[exposure][periods][window]") {
  SECTION("exposure-time-periods") {
    // Create EM so we can reference its delegate
    // Create exposure manager
    NoOptPassthrough nopt;
    DummyExposureCallbackHandlerNoOpt dh{nopt};
    herald::exposure::FixedMemoryExposureStore<8> des;
    herald::exposure::ExposureManager<DummyExposureCallbackHandlerNoOpt, herald::exposure::FixedMemoryExposureStore<8>> em(dh,des);

    bool setGlobal = em.setGlobalPeriodInterval(0,120); // 120 second windows starting at DateTime==0
    REQUIRE(setGlobal);
    REQUIRE(em.getGlobalPeriodAnchor().secondsSinceUnixEpoch() == 0);
    REQUIRE(em.getGlobalPeriodInterval().seconds() == 120);

    // Create underlying AnalysisRunner first
    SampleList<Sample<RSSI>,25> srcData;
    // Missing data items for the first minute
    srcData.push(60,-55); // one minute
    srcData.push(90,-55);
    srcData.push(120,-55); // 45
    srcData.push(150,-55);
    srcData.push(180,-55); // 90
    srcData.push(210,-55);
    srcData.push(240,-55); // 135
    srcData.push(270,-55);
    srcData.push(300,-55); // 180 (i.e. 45 * 4)
    DummySampleSource src(1234,std::move(srcData));

    herald::analysis::algorithms::RSSIMinutesAnalyser riskAnalyser{60}; // One RSSIMinute every 60 seconds of input

    auto analysisDelegate = em.template analysisDelegate<herald::datatype::RSSIMinute>(); // full object (not just reference)

    // Note: The delegate manager supports multiple analysis delegates, so you can have one per risk model type (set at compile time)
    herald::analysis::AnalysisDelegateManager adm(std::move(analysisDelegate)); // NOTE: analysisDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(riskAnalyser)); // NOTE: riskAnalyser MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<
        herald::exposure::ExposureManagerDelegate<
          herald::datatype::RSSIMinute,
          herald::exposure::ExposureManager<DummyExposureCallbackHandlerNoOpt, herald::exposure::FixedMemoryExposureStore<8>>
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

    // Now confirm callback values are the same with a different window
    REQUIRE(result); // A notification occured
    INFO("Agent from callback: " << dh.agent.string() << ", but expected: " << herald::datatype::agent::humanProximity.string());
    REQUIRE(dh.agent == herald::datatype::agent::humanProximity);
    REQUIRE(dh.called);
    REQUIRE(1 == dh.called);
    // NOTE: Currently only being fired for the FIRST exposure value in the change list
    // WARNING: This callback adds exposure no matter the start/end time period. In your Risk algorithms, scale by ACTUAL exposure start/end times
    REQUIRE(dh.currentExposureValue == (45 * 4)); // -55 (so 45) RSSI for 4 minutes (RSSIMinutes) (Started at 1 minute, but 5 minutes elapsed, so 4 windows only), RSSIMinute chops first minute anyway, meaning half for the first value

    // Now confirm we have the correct number of samples for this agent (120 second windows, starting at 60 seconds, 300 second period in total - so 2 periods)
    REQUIRE(2 == em.getCountByInstanceId(proxInstanceId));
  }
}

// [Who]   As an epidemiologist
// [What]  I need to run a risk analysis for all source variables over a given time period (E.g. hourly)
// [Value] To calculate risk values for each target period during that time (E.g. a day), to accurately estimate a variety of risks (E.g. for a screening application)

// [Who]   As a risk application developer
// [What]  I need to run risk analyses live, responding to just the source exposure data that has changed
// [Value] In order to use minimum power, memory, and data storage on the application device


TEST_CASE("risk-multi-variate", "[exposure][periods][window][risk][multi-variate]") {
  SECTION("risk-multi-variate") {
    
    // TODO Now Configure a Risk Manager, with a single algorithm taking into account indoor vs outdoor and RssiMinute values
    // Note: Simply, this uses a luminosity mean value to determine indoors vs outdoors and multiplies the risk score by the given amount
    // WARNING: THIS IS AN API SAMPLE ONLY AND IS NOT BASED ON EPIDEMIOLOGICAL STUDIES
    herald::exposure::model::SampleDiseaseScreeningRiskModel sampleRM;
    herald::exposure::RiskModels models{sampleRM};
    herald::exposure::RiskParameters<8> myStats; // decays all values to double when set
    myStats.set(herald::exposure::parameter::weight, 90.0); // I know, I know. It's referred to as weight in clinical literature.
    // See BMJ Advice on clinical medicine definition of sex (NOT gender): https://www.bmj.com/content/372/bmj.n735/rr-0
    // Also NHS Data Dictionary here: https://datadictionary.nhs.uk/classes/person_phenotypic_sex.html
    // Note that phenotypic sex is not the same as chromosonal sex either. Clinical medicine uses Phenotypic sex generally.
    myStats.set(herald::exposure::parameter::phenotypic_sex, (double)herald::datatype::phenotypic_sex::male); // male, female, indeterminate
    myStats.set(herald::exposure::parameter::age, 21.0); // Honest...
    herald::exposure::FixedMemoryRiskStore<8> dummyRiskScoreStore;
    // Note: Below is a test of the deduction guide
    herald::exposure::RiskManager<
      herald::exposure::RiskModels<herald::exposure::model::SampleDiseaseScreeningRiskModel>, 
      herald::exposure::RiskParameters<8>, 
      8
      ,
      herald::exposure::FixedMemoryRiskStore<8>
    > rm{
      std::move(models), std::move(myStats), dummyRiskScoreStore
    }; // All potential risk model classes linked at compile time (they are treated as singletons)
    rm.setGlobalPeriodInterval(Date{0}, TimeInterval::seconds(240)); // Interval every 4 minutes, just so its different to other intervals used
    // Note myStats may change over time, but are static/fixed from the point of view of a constantly running risk algorithm

    // Set the exposure store now, used by both exposure manager and risk manager
    herald::exposure::FixedMemoryExposureStore<8> des;

    using RMECA = herald::exposure::RiskManagerExposureCallbackAdapter<herald::exposure::RiskManager<
      herald::exposure::RiskModels<herald::exposure::model::SampleDiseaseScreeningRiskModel>, 
      herald::exposure::RiskParameters<8>, 
      8,
      herald::exposure::FixedMemoryRiskStore<8>
    >, herald::exposure::FixedMemoryExposureStore<8>>;
    RMECA riskExposureCallbackAdapter{rm, des};
    // herald::exposure::RiskManagerExposureCallbackAdapter riskExposureCallbackAdapter{rm}; // Note: Uses single call parameter deduction (explicit)
    // using RMECA = decltype(riskExposureCallbackAdapter);

    // Create EM so we can reference its delegate
    // Create exposure manager
    DummyExposureCallbackHandler dh{riskExposureCallbackAdapter};
    herald::exposure::ExposureManager<DummyExposureCallbackHandler<RMECA>,  herald::exposure::FixedMemoryExposureStore<8>> em(dh,des); // TODO deduction guide, THEN try 8 as first parameter to see if that silences typename warning

    bool setGlobal = em.setGlobalPeriodInterval(0,120); // 120 second windows starting at DateTime==0
    REQUIRE(setGlobal);
    REQUIRE(em.getGlobalPeriodAnchor().secondsSinceUnixEpoch() == 0);
    REQUIRE(em.getGlobalPeriodInterval().seconds() == 120);

    // Create underlying AnalysisRunner first
    SampleList<Sample<RSSI>,25> srcData;
    // Missing data items for the first minute
    srcData.push(60,-55); // one minute
    srcData.push(90,-55);
    srcData.push(120,-55); // 45
    srcData.push(150,-55);
    srcData.push(180,-55); // 90
    srcData.push(210,-55);
    srcData.push(240,-55); // 135
    srcData.push(270,-55);
    srcData.push(300,-55); // 180 (i.e. 45 * 4)
    DummySampleSource srcRssi(1234,std::move(srcData));

    // Now add dummy light level source
    SampleList<Sample<Luminosity>,25> srcLightData;
    srcLightData.push(0,5);
    srcLightData.push(30,5);
    srcLightData.push(60,5);
    srcLightData.push(90,5);
    srcLightData.push(120,5);
    srcLightData.push(150,50);
    srcLightData.push(180,50);
    srcLightData.push(210,50);
    srcLightData.push(240,50);
    srcLightData.push(270,50);
    srcLightData.push(300,50);
    // NOTE instance ID (5678) MUST be different from RSSI's as source sensors are different
    DummySampleSource srcLight(5678,std::move(srcLightData)); // Type derived from Sample List Sample's type


    herald::analysis::algorithms::RSSIMinutesAnalyser riskAnalyser{60}; // One RSSIMinute every 60 seconds of input
    herald::analysis::algorithms::RunningMeanAnalyser<herald::datatype::Luminosity, 3> lumAnalyser{120}; // Latest (max) 3 readings only used, generating data every 120 seconds
    // Note: Allowing raw light readings to pass through unaltered

    auto analysisDelegateRssi = em.template analysisDelegate<herald::datatype::RSSIMinute>(); // full object (not just reference)
    auto analysisDelegateLum = em.template analysisDelegate<herald::datatype::RunningMean<herald::datatype::Luminosity>>(); // full object (not just reference)

    // Note: The delegate manager supports multiple analysis delegates, so you can have one per risk model type (set at compile time)
    herald::analysis::AnalysisDelegateManager adm(std::move(analysisDelegateRssi),std::move(analysisDelegateLum)); // NOTE: analysisDelegate MOVED FROM and no longer accessible
    herald::analysis::AnalysisProviderManager apm(std::move(riskAnalyser), std::move(lumAnalyser)); // NOTE: risk/lumAnalysers MOVED FROM and no longer accessible

    herald::analysis::AnalysisRunner<
      herald::analysis::AnalysisDelegateManager<
        herald::exposure::ExposureManagerDelegate<
          herald::datatype::RSSIMinute,
          herald::exposure::ExposureManager<DummyExposureCallbackHandler<RMECA>, herald::exposure::FixedMemoryExposureStore<8>>
        >
        ,
        herald::exposure::ExposureManagerDelegate<
          herald::datatype::RunningMean<herald::datatype::Luminosity>,
          herald::exposure::ExposureManager<DummyExposureCallbackHandler<RMECA>, herald::exposure::FixedMemoryExposureStore<8>>
        >
      >,
      herald::analysis::AnalysisProviderManager<
        herald::analysis::algorithms::RSSIMinutesAnalyser,
        herald::analysis::algorithms::RunningMeanAnalyser<Luminosity,3>
      >,
      RSSI,RSSIMinute,Luminosity,RunningMean<Luminosity>
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
    // Now add luminosity
    herald::datatype::UUID lumInstanceId = 
      herald::datatype::UUID::fromString("88888888-1111-4011-8011-122221111111");
    // TODO determine why the following line causes changeCount to be set to 0 instead of a value
    bool addSuccess2 = em.addSource<RunningMean<Luminosity>>(
      herald::datatype::agent::lightBrightness, 
      sensorClass::luninositySingleChannelLums, lumInstanceId);
    // Note: We've linked the above to a particular Model Type (RSSIMinute), and by default are using instanceID derived from the SampledId
    REQUIRE(addSuccess1);
    REQUIRE(addSuccess2);
    REQUIRE(em.sourceCount() == 2);

    // Note that for this data, and 120 second Exposure periods, we should have:-
    // RunningMean<Light>:-
    // 0 (@120 secs) = 5 (last three were each 5)
    // 1 (@240 secs) = 50 (last 3 were each 50)
    // 2 (@360) seconds = 50 (Only two, both 50. Because we only have data for 60 seconds, but it's a MAX of last 1-3 algorithm)
    // lumExposure scores:-
    // Not recorded (120-120) = 5
    // 1 (120-240) = 50
    // 2 (240-360) = 50






    // TODO FIX THE ABOVE SO OUR data analysis interval is INDEPENDENT of our generation period interval






    // RSSIMinute:-
    // not given (60 secs) = 0 (timeDelta is 0)
    // 0 (120 secs) = 45
    // 1 (180 secs) = 45
    // 2 (240 secs) = 45
    // 3 (300 secs) = 45
    // humanProx exposure (Mean of RSSIMinutes):-
    // Not recorded (120-120 secs) = 0 (Because we have data from 120 seconds only, as RSSIMinute itself is from an aggregate, and we're not yet accounting for difference in periodEnd and periodStart)
    // 1 (120-240 secs) = 45 * 2 = 90 (2 minutes)
    // 2 (240-360 secs) = 90 (45 *2  /2 (DIVISION NOT APPLIED YET)) (Because we only have data for 60 seconds (240-300 secs), and we're not yet accounting for difference in periodEnd and periodStart)
    
    // This should result in the following from the simple Risk Model we have with 240 second Risk periods:-
    // RiskScore = Sum (humanProx) * age (Parameter = 21) * multiplier (1 if max(light) > 100, 2 if 1 < max(light) < 100) 
    // 0 (0-240 secs) = 67.5 * 21.0 * 2 (Max light(of last 3) = 50) = 2835
    // 1 (240-480 secs) = 22.5 * 21.0 * 2 (Max light(of last 3) = 25) = 945
    // Simple total (Calculated by simple external aggregation below - NOT the 'total risk score' as per the model) = 3780
    // ACTUAL right now:-
    // 0 = 180 * 21 * 2
    // 1 = 100 * 21 * 2
    // total = 11760
    // reported total is 15120!!!
    
    // We can have multiple risk model instances with different config for the same variables
    herald::datatype::UUID sampleRMID = 
      herald::datatype::UUID::fromString("77777777-1111-4011-8011-134341111111");
    herald::datatype::Agent riskModelOutputAgent{55}; // some nominal output agent ID
    bool addedRM = rm.addRiskModel(sampleRMID, riskModelOutputAgent, sampleRM); // empty configuration to pass to it
    REQUIRE(addedRM);
    // rm.addExposureSource(em);

    // Now run the values through
    em.enableRunning(); // required, else no changes will be recorded
    // WARNING: Unlike in real life, these data add operations occur in series, not in parallel
    srcRssi.run(301, runner); // NB correctly increments changeCount (to 1, which is modified throughout)
    srcLight.run(301, runner);
    // Now fire off any exposure changes
    bool result = em.notifyOfChanges(); // Note: Was failing because of substitution failure in analysis API due to SampleList iterator not supporting const
    // The above should update the Risk Manager (rm) once too

    // Now confirm callback values are the same with a different window
    REQUIRE(result); // A notification occured
    // Note: Not introspecting callbacks as multiple variables which are valid to arrive in any order

    // Now confirm we have the correct number of samples for each agent
    // RSSI: 120 second windows, starting at 60 seconds, 300 second period in total - so 2 periods
    // Luminosity: 60 second windows, starting at 0 seconds, 300 second period in total, 300 second period in total - so
    REQUIRE(2 == em.getCountByInstanceId(lumInstanceId));
    // Check luminosity hasn't overwritten rssi prox values
    auto cnt = em.getCountByInstanceId(proxInstanceId);
    REQUIRE(2 == cnt);

    // Check values of aggregated exposure data first
    // Prox intances first
    cnt = 0;
    bool passed = true;
    em.forEachExposure(proxInstanceId,[&passed, &cnt] (const herald::datatype::ExposureMetadata& meta, const Exposure& score) -> void {
      if (0 == cnt) {
        // if (22.5 != score.value) {
        if (90.0 != score.value) {
          passed = false;
          std::cout << "Element 0 should be 90 but is actually " << score.value << std::endl;
        }
      } else if (1 == cnt) {
        // if (45.0 != score.value) {
        if (90.0 != score.value) {
          passed = false;
          std::cout << "Element 1 should be 90 but is actually " << score.value << std::endl;
        }
      // } else {
      //   if (22.5 != score.value) {
      //     passed = false;
      //     std::cout << "Element 2 should be 22.5 but is actually " << score.value << std::endl;
        // }
      }
      ++cnt;
    });
    REQUIRE(2 == cnt);
    REQUIRE(passed);

    // Now do the same for Light exposure which is Max(Latest 3 light)
    cnt = 0;
    passed = true;
    em.forEachExposure(lumInstanceId,[&passed, &cnt] (const herald::datatype::ExposureMetadata& meta, const Exposure& score) -> void {
      if (0 == cnt) {
        if (5.0 != score.value) {
          passed = false;
          std::cout << "Element 0 should be 5 but is actually " << score.value << std::endl;
        }
        // ONLY ONE BECAUSE WE RAN EXPOSURE MANAGER AND ANALYSIS API UP TO 301 SECONDS ONLY, NOT 360 AS REQUIRED
        // CHANGED TO 361 AND THUS 2
      } else if (1 == cnt) {
        if (50.0 != score.value) {
          passed = false;
          std::cout << "Element 1 should be 50 but is actually " << score.value << std::endl;
        }
      // } else {
      //   if (50.0 != score.value) {
      //     passed = false;
      //     std::cout << "Element 2 should be 50 but is actually " << score.value << std::endl;
      //   }
      }
      ++cnt;
    });
    REQUIRE(passed);

    // Also determine that there are two risk values for the same time period (risk times are set to 120 seconds)
    // Risk score should be:-
    // - for first 60 seconds, 30 (our base level, at indoor lighting)
    // - for next 60 seconds, 45 (actual RSSI, at indoor lighting)
    // - then for remaining time, 45 * 2 (actual RSSI, at outdoor lighting) (note: our mean outside level of 10 is below 5 + 5 + 50 / 3, our first 'outside' lum values, so immediately becomes outdoors)
    // - today's total therefore (30 + 45 + 90 + 90) = 255
    herald::datatype::Date riskAnchorDate{0};
    herald::datatype::TimeInterval riskInterval = TimeInterval::seconds(240);
    REQUIRE(rm.getGlobalPeriodAnchor() == riskAnchorDate);
    REQUIRE(rm.getGlobalPeriodInterval() == riskInterval);
    REQUIRE(2 == rm.getRiskScoreCount(sampleRMID)); // 0-300 seconds with 240 seconds interval, indexed at 0 seconds, means 2 output risk scores, not 4 (like in exposure metadata)

    RiskScore total;
    std::size_t callCount = 0;
    bool setTotal = false;
    bool rmCallableResult = rm.forEachRiskScore(sampleRMID, [&total,&setTotal,&callCount] (const herald::datatype::RiskScoreMetadata& meta, const herald::datatype::RiskScore& score) -> bool {
      // NOTE I'm not checking the meta passed, as I know in this test it's the only RiskMetadata in use for this callback
      if (!setTotal) {
        total = score;
        setTotal = true; // TODO consider if calling += on an uninitialised RiskScore should set the risk metadata as well as the score itself
      } else {
        total += score;
      }
      ++callCount;

      // continue to return results
      return true;
    });
    REQUIRE(rmCallableResult);
    REQUIRE(2 == callCount);
    REQUIRE(4500 == total.value); // TODO consider approximate bounds for double value
  }
}

// [Who]   
// [What]  
// [Value] 

// [Who]   
// [What]  
// [Value] 

// [Who]   As a risk application user
// [What]  I need to record static (E.g. phylogenic sex) and variable (E.g. weight) biographic data over a long time period
// [Value] To ensure risk algorithms I'm relying on can take these factors in to account

// [Who]   As a risk application provider
// [What]  I need to track the finite states of a user from values in a risk model over time
// [Value] In order to provide timely and useful health advice based on risk analyses


// NOTE THE FOLLOWING ARE FOR A FUTURE RISK SCORE MANAGER - REFACTORING FROM OLD EXPOSURE MANAGER CLASS

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

