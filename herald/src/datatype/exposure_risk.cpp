//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/exposure_risk.h"

namespace herald {
namespace datatype {

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
    sensorClassId == other.sensorClassId
    // Note: Do not use sensor instance id for comparison
  );
}
const bool
ExposureMetadata::operator!=(const ExposureMetadata& other) const noexcept
{
  return (
    agentId != other.agentId ||
    sensorClassId != other.sensorClassId
    // Note: Do not use sensor instance id for comparison
  );
}







const bool
RiskScoreMetadata::operator==(const RiskScoreMetadata& other) const noexcept
{
  return (
    agentId == other.agentId &&
    algorithmId == other.algorithmId
  );
}

const bool
RiskScoreMetadata::operator!=(const RiskScoreMetadata& other) const noexcept
{
  return (
    agentId != other.agentId ||
    algorithmId != other.algorithmId
  );
}


}
}