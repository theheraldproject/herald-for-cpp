//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_RISK_H
#define HERALD_EXPOSURE_RISK_H

#include "date.h"
#include "uuid.h"
#include "allocatable_array.h"

namespace herald {
namespace datatype {

/**
 * /brief Represents the common data items between Exposure and Rirk Scores
 */
struct Score {
  Date periodStart; // defaults to "now"
  Date periodEnd; // defaults to "now"
  double value = 0.0;
  double confidence = 1.0;

  const bool operator==(const Score& other) const noexcept;
  const bool operator!=(const Score& other) const noexcept;
  Score operator+(const Score& other) const noexcept;
};

/**
 * /brief Represents a measurable exposure to an agent from an individual sensor.
 *
 * This is different from a raw sample as it represents an aggregated value over a period
 * of time. As an example, a set of proximity sensor readings may aggregate into a
 * proximity "metre minute" score overtime. This is just an example. Any measure is possible.
 */
using Exposure = Score;

struct ExposureMetadata {
  UUID agentId = UUID::unknown();
  UUID sensorClassId = UUID::unknown();
  UUID sensorInstanceId = UUID::unknown();

  const bool operator==(const ExposureMetadata& other) const noexcept;
  const bool operator!=(const ExposureMetadata& other) const noexcept;
};

template <std::size_t ArraySize=1>
using ExposureArray = TaggedArray<
  ExposureMetadata,Exposure,ArraySize,false
>;

/**
 * /brief A set of arrays linking a single exposure source to exposure readings
 * 
 * Defaults to exposure size of 1 as the assumption is we're adding to the exposure live
 */
template <std::size_t ExposureArraySize=1,std::size_t MaxTags=8>
using ExposureSet = TaggedArraySet<
  ExposureMetadata,Exposure,ExposureArraySize,false,
  MaxTags,true
>;





/**
 * /brief Represents an estimated Risk score through calculation, typically from a mix of factors
 *        including data about an individual, and exposures from the environment measured on device.
 *
 * This is different from an exposure in that it uses exposure data which may be supplied continuously
 * for short periods of time per exposure value, and calculates a risk score from these source variables.
 * There may be many risk score algorithms that run over a different combination of variables. This struct
 * represents the result (which may be comparable to results from other risk score algorithms) and not
 * the metadata on how the score was calculated. That is held in the generating algorithm class, and
 * managed by the Risk Manager.
 */
using RiskScore = Score;

struct RiskScoreMetadata {
  UUID agentId = UUID::unknown();
  UUID algorithmId = UUID::unknown();

  const bool operator==(const RiskScoreMetadata& other) const noexcept;
  const bool operator!=(const RiskScoreMetadata& other) const noexcept;
};


template <std::size_t ArraySize=1>
using RiskScoreArray = TaggedArray<
  RiskScoreMetadata,RiskScore,ArraySize,false
>;

/**
 * /brief A set of arrays linking a single risk algorithm for an agent to its results
 * 
 * Defaults to risk score size of 1 as the assumption is we're modifying the risk score live
 */
template <std::size_t RiskScoreArraySize=1,std::size_t MaxTags=8>
using RiskScoreSet = TaggedArraySet<
  RiskScoreMetadata,RiskScore,RiskScoreArraySize,false,
  MaxTags,true
>;



}
}

#endif