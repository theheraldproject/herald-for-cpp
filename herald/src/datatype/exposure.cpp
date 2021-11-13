//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/exposure.h"

namespace herald {
namespace datatype {

const bool
Exposure::operator==(const Exposure& other) const noexcept
{
  return (
    (agentId == other.agentId) &&
    (sensorClassId == other.sensorClassId) &&
    (sensorInstanceId == other.sensorInstanceId) &&
    (periodStart == other.periodStart) &&
    (periodEnd == other.periodEnd) &&
    (value == other.value) &&
    (confidence == other.confidence)
  );
}

const bool
Exposure::operator!=(const Exposure& other) const noexcept
{
  return (
    (agentId != other.agentId) ||
    (sensorClassId != other.sensorClassId) ||
    (sensorInstanceId != other.sensorInstanceId) ||
    (periodStart != other.periodStart) ||
    (periodEnd != other.periodEnd) ||
    (value != other.value) ||
    (confidence != other.confidence)
  );
}


Exposure
Exposure::operator+(const Exposure& other) const noexcept
{
  return Exposure{
    .agentId = agentId,
    .sensorClassId = sensorClassId,
    .sensorInstanceId = sensorInstanceId,
    .periodStart = (periodStart < other.periodStart ? periodStart : other.periodStart),
    .periodEnd = (periodEnd > other.periodEnd ? periodEnd : other.periodEnd),
    .value = value + other.value,
    .confidence = confidence < other.confidence ? confidence : other.confidence
  };
}

}
}
