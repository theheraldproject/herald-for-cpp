//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/exposure_risk.h"

namespace herald {
namespace datatype {


Score&
Score::operator=(const Score& other) noexcept
{
  value = other.value;
  periodStart = other.periodStart;
  periodEnd = other.periodEnd;
  confidence = other.confidence;
  return *this;
}

const bool
Score::operator==(const Score& other) const noexcept
{
  return (
    periodStart == other.periodStart &&
    periodEnd == other.periodEnd &&
    value == other.value &&
    confidence == other.confidence
  );
}

const bool
Score::operator!=(const Score& other) const noexcept
{
  return  (
    periodStart != other.periodStart ||
    periodEnd != other.periodEnd ||
    value != other.value ||
    confidence != other.confidence
  );
}

Score
Score::operator+(const Score& other) noexcept
{
  // TODO support scaling of confidence
  return Score{
    .periodStart = (periodStart > other.periodStart ? other.periodStart : periodStart),
    .periodEnd = (periodEnd < other.periodEnd ? other.periodEnd : periodEnd),
    .value = value + other.value,
    .confidence = confidence
  };
}

Score&
Score::operator+=(const Score& other) noexcept
{
  // TODO support scaling of confidence
  value += other.value;
  if (periodStart > other.periodStart) {
    periodStart = other.periodStart;
  }
  if (periodEnd < other.periodEnd) {
    periodEnd = other.periodEnd;
  }
  return *this;
}





const bool
ExposureMetadata::operator==(const ExposureMetadata& other) const noexcept
{
  return (
    agentId == other.agentId &&
    sensorClassId == other.sensorClassId &&
    modelClassId == other.modelClassId // TODO determine if this should be for all models too (i.e. multiple sources with same agentId and sensorClassId are 'equal'?)
    // Note: Do not use sensor instance id for comparison
  );
}

const bool
ExposureMetadata::operator!=(const ExposureMetadata& other) const noexcept
{
  return (
    agentId != other.agentId ||
    sensorClassId != other.sensorClassId ||
    modelClassId != other.modelClassId
    // Note: Do not use sensor instance id for comparison
  );
}







const bool
RiskScoreMetadata::operator==(const RiskScoreMetadata& other) const noexcept
{
  return (
    agentId == other.agentId &&
    algorithmId == other.algorithmId &&
    instanceId == other.instanceId
  );
}

const bool
RiskScoreMetadata::operator!=(const RiskScoreMetadata& other) const noexcept
{
  return (
    agentId != other.agentId ||
    algorithmId != other.algorithmId ||
    instanceId != other.instanceId
  );
}


}
}