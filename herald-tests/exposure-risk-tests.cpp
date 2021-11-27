//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "herald/herald.h"

using namespace herald::analysis::sampling;
using namespace herald::datatype;

//   [Who] As a sensor device user
//  [What] I need to track aggregated risk exposures to different agents
// [Value] In order to inform direct exposure decisions, or downstream risk analyses

//   [Who] As an exposure application developer
//  [What] I need to link exposure to the agent, sensor type, and local sensor instance
// [Value] In order to inform decisions on storing and managing exposure aggregate data

//   [Who] As an exposure application developer
//  [What] I need to record the period (start and end date time) for exposures
// [Value] In order to make decisions about further aggregation, and track when I have exposure data for

//   [Who] As an exposure application developer
//  [What] I need to support multiple devices providing exposure data for the same agent
// [Value] In order to allow aggregation of exposure from multiple wearable, carryable, and environment (IoT) sensor sources

//   [Who] As an exposure application developer
//  [What] I need to quantify exposure as a double precision continuous variable
// [Value] In order to allow accurate recording and addition for exposure aggregation

//   [Who] As an exposure application developer
//  [What] I need to store the confidence (accuracy/probability) of the reading from the sensor instance source
// [Value] In order to allow downstream decisions on total exposure and risk scoring confidence

// TODO decide if the above is better expressed as a percentage +/- 
//      accuracy / probability, or as a CI 95% value with different +/- values

// TODO decide if agentId, sensorClassId, and sensorInstanceId should be in a separate
//      struct attached to the exposure (sample) list, rather than within the exposure itself

TEST_CASE("score-default", "[exposure][default]") {
  SECTION("score-default") {
    // Test default exposure class is valid
    Date now;
    Score exp;
    UUID unk = UUID::unknown();
    // REQUIRE(unk == exp.agentId);
    // REQUIRE(unk == exp.sensorClassId);
    // REQUIRE(unk == exp.sensorInstanceId);
    REQUIRE(now <= exp.periodStart);
    REQUIRE(now <= exp.periodEnd);
    REQUIRE(exp.periodEnd >= exp.periodStart);
    REQUIRE(0.0 == exp.value);
    REQUIRE(1.0 == exp.confidence);
  }
}

TEST_CASE("score-filled", "[exposure][filled]") {
  SECTION("score-filled") {
    // Test filled exposure class is valid
    // UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    // UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    // UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Score exp{
      // .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      // .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      // .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };
    // Test our test variables first
    UUID unk = UUID::unknown();
    // REQUIRE(unk != exp.agentId);
    // REQUIRE(unk != exp.sensorClassId);
    // REQUIRE(unk != exp.sensorInstanceId);
    // REQUIRE(exp.agentId != exp.sensorClassId);
    // REQUIRE(exp.sensorClassId != exp.sensorInstanceId);
    // REQUIRE(exp.sensorInstanceId != exp.agentId);

    // Now test their use
    // REQUIRE(agent == exp.agentId);
    // REQUIRE(sensorClass == exp.sensorClassId);
    // REQUIRE(sensorInstance == exp.sensorInstanceId);
    REQUIRE(Date(4000) == exp.periodStart);
    REQUIRE(Date(5000) == exp.periodEnd);
    REQUIRE(exp.periodEnd > exp.periodStart);
    REQUIRE(5.6 == exp.value);
    REQUIRE(0.68 == exp.confidence);
  }
}

TEST_CASE("score-copyctor", "[exposure][copyctor]") {
  SECTION("score-copyctor") {
    // UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    // UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    // UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    const Score exp{
      // .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      // .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      // .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };

    // Copy construction operation
    Score exp2 = exp; // copy constructor (const copy ctor forced)
    
    // REQUIRE(exp.agentId == exp2.agentId);
    // REQUIRE(exp.sensorClassId == exp2.sensorClassId);
    // REQUIRE(exp.sensorInstanceId == exp2.sensorInstanceId);
    REQUIRE(exp.periodStart == exp2.periodStart);
    REQUIRE(exp.periodEnd == exp2.periodEnd);
    REQUIRE(exp.value == exp2.value);
    REQUIRE(exp.confidence == exp2.confidence);

    // Now test full equality operator
    REQUIRE(exp == exp2);
    REQUIRE(!(exp != exp2));
  }
}

TEST_CASE("score-movector", "[exposure][movector]") {
  SECTION("score-movector") {
    // Test filled exposure class is valid
    // UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    // UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    // UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Score expMovedFrom{
      // .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      // .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      // .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };

    // move constructor operation
    Score exp(std::move(expMovedFrom)); // force move ctor

    // Test our test variables first
    UUID unk = UUID::unknown();
    // REQUIRE(unk != exp.agentId);
    // REQUIRE(unk != exp.sensorClassId);
    // REQUIRE(unk != exp.sensorInstanceId);
    // REQUIRE(exp.agentId != exp.sensorClassId);
    // REQUIRE(exp.sensorClassId != exp.sensorInstanceId);
    // REQUIRE(exp.sensorInstanceId != exp.agentId);

    // Now test their use
    // REQUIRE(agent == exp.agentId);
    // REQUIRE(sensorClass == exp.sensorClassId);
    // REQUIRE(sensorInstance == exp.sensorInstanceId);
    REQUIRE(Date(4000) == exp.periodStart);
    REQUIRE(Date(5000) == exp.periodEnd);
    REQUIRE(exp.periodEnd > exp.periodStart);
    REQUIRE(5.6 == exp.value);
    REQUIRE(0.68 == exp.confidence);

    // Test validity but difference of moved from object
    // WARNING do not do this - expMovedFrom MAY be the same
    // REQUIRE(expMovedFrom != exp);
  }
}

TEST_CASE("score-addition", "[exposure][addition]") {
  SECTION("score-addition") {
    // Test filled exposure class is valid
    // UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    // UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    // UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Score exp1{
      // .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      // .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      // .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };
    Score exp2{
      // .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      // .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      // .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{5500},
      .periodEnd = Date{6500},
      .value = 3.5,
      .confidence = 0.7 // As this is a probability this should be bounded
    };

    REQUIRE(exp1 != exp2);
    REQUIRE(!(exp1 == exp2));

    // addition operation
    Score added = exp1 + exp2;

    // REQUIRE(agent == added.agentId);
    // REQUIRE(sensorClass == added.sensorClassId);
    // REQUIRE(sensorInstance == added.sensorInstanceId);
    REQUIRE(Date(4000) == added.periodStart);
    REQUIRE(Date(6500) == added.periodEnd);
    REQUIRE(added.periodEnd > added.periodStart);
    REQUIRE(9.1 == added.value);
    // TODO decide what to do about confidence score addition (pick lowest for now)
    REQUIRE(0.68 == added.confidence);
  }
}


/// MARK: Exposure Metadata Tests

TEST_CASE("exposure-metadata-default", "[exposure][metadata][default") {
  SECTION("exposure-metadata-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    ExposureMetadata em1{
      .agentId = agent,
      .sensorClassId = sensorClass,
      .sensorInstanceId = sensorInstance
    };
    ExposureMetadata em2{
      .agentId = agent,
      .sensorClassId = sensorClass,
      .sensorInstanceId = sensorInstance
    };
    REQUIRE(em1.agentId == agent);
    REQUIRE(em1.sensorClassId == sensorClass);
    REQUIRE(em1.sensorInstanceId == sensorInstance);
    REQUIRE(em2.agentId == agent);
    REQUIRE(em2.sensorClassId == sensorClass);
    REQUIRE(em2.sensorInstanceId == sensorInstance);
    REQUIRE(em1 == em2);
    REQUIRE(!(em1 != em2));
  }
}



/// MARK: Exposure Array Tests

TEST_CASE("exposure-array-default", "[exposure][array][allocatable-array][default") {
  SECTION("exposure-array-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    ExposureMetadata em{
      .agentId = agent,
      .sensorClassId = sensorClass,
      .sensorInstanceId = sensorInstance
    };
    ExposureArray<8> exposures(em);

    REQUIRE(exposures.getTag().agentId == agent);
    REQUIRE(exposures.getTag().sensorClassId == sensorClass);
    REQUIRE(exposures.getTag().sensorInstanceId == sensorInstance);
    REQUIRE(ExposureArray<8>::max_size == 8);
    REQUIRE(exposures.contents().size() == 0);
  }
}


TEST_CASE("exposure-array-set-default", "[exposure][array][set][allocatable-array][default") {
  SECTION("exposure-array-set-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    ExposureMetadata em{
      .agentId = agent,
      .sensorClassId = sensorClass,
      .sensorInstanceId = sensorInstance
    };
    ExposureSet<8,16> es;

    REQUIRE(es.size() == 0);
    REQUIRE(ExposureSet<8,16>::max_size == 16); // outer set size

    es.add(ExposureArray<8>(em));

    REQUIRE(es.size() == 1);
    REQUIRE(es[0].getTag() == em);
    REQUIRE(!(es[0].getTag() != em));
    REQUIRE(es[0].contents().size() == 0); // inner tagged array size
  }
}

/// MARK: Risk Score Metadata Tests

TEST_CASE("risk-score-metadata-default", "[risk-score][metadata][default") {
  SECTION("risk-score-metadata-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID algorithm = UUID::fromString("51111111-1111-4011-8011-111111111111");
    RiskScoreMetadata rsm1{
      .agentId = agent,
      .algorithmId = algorithm
    };
    RiskScoreMetadata rsm2{
      .agentId = agent,
      .algorithmId = algorithm
    };
    REQUIRE(rsm1.agentId == agent);
    REQUIRE(rsm1.algorithmId == algorithm);
    REQUIRE(rsm2.agentId == agent);
    REQUIRE(rsm2.algorithmId == algorithm);
    REQUIRE(rsm1 == rsm2);
    REQUIRE(!(rsm1 != rsm2));
  }
}

/// MARK: Risk Score Array Tests

TEST_CASE("risk-score-array-default", "[risk-score][array][allocatable-array][default") {
  SECTION("risk-score-array-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID algorithm = UUID::fromString("51111111-1111-4011-8011-111111111111");
    RiskScoreMetadata rsm{
      .agentId = agent,
      .algorithmId = algorithm
    };
    RiskScoreArray<8> scores(rsm);

    REQUIRE(scores.getTag().agentId == agent);
    REQUIRE(scores.getTag().algorithmId == algorithm);
    REQUIRE(RiskScoreArray<8>::max_size == 8);
    REQUIRE(scores.contents().size() == 0);
    REQUIRE(scores.getTag() == rsm);
    REQUIRE(!(scores.getTag() != rsm));
  }
}

TEST_CASE("risk-score-array-set-default", "[risk-score][array][set][allocatable-array][default") {
  SECTION("risk-score-array-set-default") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID algorithm = UUID::fromString("51111111-1111-4011-8011-111111111111");
    RiskScoreMetadata rsm{
      .agentId = agent,
      .algorithmId = algorithm
    };
    RiskScoreSet<8,16> rss;

    REQUIRE(rss.size() == 0);
    REQUIRE(RiskScoreSet<8,16>::max_size == 16); // outer set size

    rss.add(RiskScoreArray<8>(rsm));

    REQUIRE(rss.size() == 1);
    REQUIRE(rss[0].getTag().agentId == agent);
    REQUIRE(rss[0].getTag().algorithmId == algorithm);
    REQUIRE(rss[0].getTag() == rsm);
    REQUIRE(!(rss[0].getTag() != rsm));
    REQUIRE(rss[0].contents().size() == 0); // inner tagged array size
  }
}