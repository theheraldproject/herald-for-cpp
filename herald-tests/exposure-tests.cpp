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

TEST_CASE("exposure-default", "[exposure][default]") {
  SECTION("exposure-default") {
    // Test default exposure class is valid
    Date now;
    Exposure exp;
    UUID unk = UUID::unknown();
    REQUIRE(unk == exp.agentId);
    REQUIRE(unk == exp.sensorClassId);
    REQUIRE(unk == exp.sensorInstanceId);
    REQUIRE(now <= exp.periodStart);
    REQUIRE(now <= exp.periodEnd);
    REQUIRE(exp.periodEnd >= exp.periodStart);
    REQUIRE(0.0 == exp.value);
    REQUIRE(1.0 == exp.confidence);
  }
}

TEST_CASE("exposure-filled", "[exposure][filled]") {
  SECTION("exposure-filled") {
    // Test filled exposure class is valid
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Exposure exp{
      .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };
    // Test our test variables first
    UUID unk = UUID::unknown();
    REQUIRE(unk != exp.agentId);
    REQUIRE(unk != exp.sensorClassId);
    REQUIRE(unk != exp.sensorInstanceId);
    REQUIRE(exp.agentId != exp.sensorClassId);
    REQUIRE(exp.sensorClassId != exp.sensorInstanceId);
    REQUIRE(exp.sensorInstanceId != exp.agentId);

    // Now test their use
    REQUIRE(agent == exp.agentId);
    REQUIRE(sensorClass == exp.sensorClassId);
    REQUIRE(sensorInstance == exp.sensorInstanceId);
    REQUIRE(Date(4000) == exp.periodStart);
    REQUIRE(Date(5000) == exp.periodEnd);
    REQUIRE(exp.periodEnd > exp.periodStart);
    REQUIRE(5.6 == exp.value);
    REQUIRE(0.68 == exp.confidence);
  }
}

TEST_CASE("exposure-copyctor", "[exposure][copyctor]") {
  SECTION("exposure-copyctor") {
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    const Exposure exp{
      .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };

    // Copy construction operation
    Exposure exp2 = exp; // copy constructor (const copy ctor forced)
    
    REQUIRE(exp.agentId == exp2.agentId);
    REQUIRE(exp.sensorClassId == exp2.sensorClassId);
    REQUIRE(exp.sensorInstanceId == exp2.sensorInstanceId);
    REQUIRE(exp.periodStart == exp2.periodStart);
    REQUIRE(exp.periodEnd == exp2.periodEnd);
    REQUIRE(exp.value == exp2.value);
    REQUIRE(exp.confidence == exp2.confidence);

    // Now test full equality operator
    REQUIRE(exp == exp2);
  }
}

TEST_CASE("exposure-movector", "[exposure][movector]") {
  SECTION("exposure-movector") {
    // Test filled exposure class is valid
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Exposure expMovedFrom{
      .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };

    // move constructor operation
    Exposure exp(std::move(expMovedFrom)); // force move ctor

    // Test our test variables first
    UUID unk = UUID::unknown();
    REQUIRE(unk != exp.agentId);
    REQUIRE(unk != exp.sensorClassId);
    REQUIRE(unk != exp.sensorInstanceId);
    REQUIRE(exp.agentId != exp.sensorClassId);
    REQUIRE(exp.sensorClassId != exp.sensorInstanceId);
    REQUIRE(exp.sensorInstanceId != exp.agentId);

    // Now test their use
    REQUIRE(agent == exp.agentId);
    REQUIRE(sensorClass == exp.sensorClassId);
    REQUIRE(sensorInstance == exp.sensorInstanceId);
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

TEST_CASE("exposure-addition", "[exposure][addition]") {
  SECTION("exposure-addition") {
    // Test filled exposure class is valid
    UUID agent = UUID::fromString("11111111-1111-4011-8011-111111111111");
    UUID sensorClass = UUID::fromString("21111111-1111-4011-8011-111111111111");
    UUID sensorInstance = UUID::fromString("31111111-1111-4011-8011-111111111111");
    Exposure exp1{
      .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{4000},
      .periodEnd = Date{5000},
      .value = 5.6,
      .confidence = 0.68 // As this is a probability this should be bounded
    };
    Exposure exp2{
      .agentId = UUID::fromString("11111111-1111-4011-8011-111111111111"),
      .sensorClassId = UUID::fromString("21111111-1111-4011-8011-111111111111"),
      .sensorInstanceId = UUID::fromString("31111111-1111-4011-8011-111111111111"),
      .periodStart = Date{5500},
      .periodEnd = Date{6500},
      .value = 3.5,
      .confidence = 0.7 // As this is a probability this should be bounded
    };

    // addition operation
    Exposure added = exp1 + exp2;

    REQUIRE(agent == added.agentId);
    REQUIRE(sensorClass == added.sensorClassId);
    REQUIRE(sensorInstance == added.sensorInstanceId);
    REQUIRE(Date(4000) == added.periodStart);
    REQUIRE(Date(6500) == added.periodEnd);
    REQUIRE(added.periodEnd > added.periodStart);
    REQUIRE(9.1 == added.value);
    // TODO decide what to do about confidence score addition (pick lowest for now)
    REQUIRE(0.68 == added.confidence);
  }
}
