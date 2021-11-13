//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_H
#define HERALD_EXPOSURE_H

#include "date.h"
#include "uuid.h"

namespace herald {
namespace datatype {

/**
 * /brief Represents a measurable exposure to an agent from an individual sensor.
 *
 * This is different from a raw sample as it represents an aggregated value over a period
 * of time. As an example, a set of proximity sensor readings may aggregate into a
 * proximity "metre minute" score overtime. This is just an example. Any measure is possible.
 */
struct Exposure {
  UUID agentId = UUID::unknown();
  UUID sensorClassId = UUID::unknown();
  UUID sensorInstanceId = UUID::unknown();
  Date periodStart; // defaults to "now"
  Date periodEnd; // defaults to "now"
  double value = 0.0;
  double confidence = 1.0;

  const bool operator==(const Exposure& other) const noexcept;
  const bool operator!=(const Exposure& other) const noexcept;
  Exposure operator+(const Exposure& other) const noexcept;
};

// TODO decide if the first three above should be in a separate
//      ExposureMetadata struct to allow more efficient in-memory storage

}
}

#endif