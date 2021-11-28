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

/** /brief Specific strongly typed override of UUID for Agents **/
struct Agent : public UUID {
public:
  constexpr Agent(UUID::value_type shortCode) noexcept
   : UUID(std::array<value_type, max_size>{0})
  {
    // Copy short code to first position
    mData[0] = shortCode;
    // Copy agent flags into last position
    // Copy V4 valid UUID into other positions
    constexpr value_type M = 0x40; // 7th byte = 0100 in binary for MSB 0000 for LSB - v4 UUID
    constexpr value_type N = 0x80; // 9th byte = 1000 in binary for MSB 0000 for LSB - variant 1
    mData[6] = (0x0f & mData[6]) | M; // blanks out first 4 bits
    mData[8] = (0x3f & mData[8]) | N; // blanks out first 2 bits

    mData[max_size - 1] = value_type{1}; // explicit construction

    mValid = true;
  }

  // constexpr Agent& operator=(const UUID::value_type shortCode) noexcept;
  ~Agent() noexcept = default;
};

/** /brief Specific strongly types override of UUID for Sensor Calsses **/
struct SensorClass : public UUID {
public:
  constexpr SensorClass(UUID::value_type shortCode) noexcept
   : UUID(std::array<value_type, max_size>{0})
  {
    // Copy short code to first position
    mData[0] = shortCode;
    // Copy agent flags into last position
    // Copy V4 valid UUID into other positions
    constexpr value_type M = 0x40; // 7th byte = 0100 in binary for MSB 0000 for LSB - v4 UUID
    constexpr value_type N = 0x80; // 9th byte = 1000 in binary for MSB 0000 for LSB - variant 1
    mData[6] = (0x0f & mData[6]) | M; // blanks out first 4 bits
    mData[8] = (0x3f & mData[8]) | N; // blanks out first 2 bits

    mData[max_size - 1] = value_type{2}; // explicit construction

    mValid = true;
  }

  ~SensorClass() noexcept = default;
};

/// /brief Agent common UUIDs for cross-compatibility (may not be a comprehensive list)
namespace agent {
  /*** /brief Human proximity agent class **/
  static constexpr Agent humanProximity{1};
  /** /brief Single channel visible light luminosity **/
  static constexpr Agent lightBrightness{2};
  /** /brief Four channel visible light plus Infra Red luminosity **/
  static constexpr Agent lightRGBIR{3};
  /** /brief Radiation exposure **/
  static constexpr Agent radiation{4};
  /** /brief Sound volume exposure **/
  static constexpr Agent sound{5};
}

/**
 * /brief Classes of sensors. Multiple sensor types may provide data on the same agent
 */
namespace sensorClass {
  /** /brief Herald Bluetooth proximity sensor **/
  static constexpr SensorClass bluetoothProximityHerald{1};
  /** /brief Legacy OpenTrace (V1 ONLY) Bluetooth proximity sensor **/
  static constexpr SensorClass bluetoothProximityOpenTrace{2};
  /** /brief Legacy Gooigle Apple Exposure Notification (GAEN) sensor **/
  static constexpr SensorClass bluetoothProximityGaen{3};
}



/**
 * /brief Represents the common data items between Exposure and Rirk Scores
 */
struct Score {
  Date periodStart; // defaults to "now"
  Date periodEnd; // defaults to "now"
  double value = 0.0;
  double confidence = 1.0;

  Score& operator=(const Score& other) noexcept;
  const bool operator==(const Score& other) const noexcept;
  const bool operator!=(const Score& other) const noexcept;
  Score operator+(const Score& other) noexcept;
  Score& operator+=(const Score& other) noexcept;
};

/**
 * /brief Represents a measurable exposure to an agent from an individual sensor.
 *
 * This is different from a raw sample as it represents an aggregated value over a period
 * of time. As an example, a set of proximity sensor readings may aggregate into a
 * proximity "metre minute" score overtime. This is just an example. Any measure is possible.
 */
using Exposure = Score;

/**
 * @brief Reppresents the metadata associated with a set of Exposure reading values.
 */
struct ExposureMetadata {
  UUID agentId = UUID::unknown();
  UUID sensorClassId = UUID::unknown();
  UUID sensorInstanceId = UUID::unknown();
  UUID modelClassId = UUID::unknown();

  const bool operator==(const ExposureMetadata& other) const noexcept;
  const bool operator!=(const ExposureMetadata& other) const noexcept;
};

/**
 * @brief Represents an array of exposures, with the 'Tag' being ExposureMetadata
 * 
 * @tparam ArraySize The maximum number of values to store against each ExposureMetadata instance. Defaulted to one (i.e. the current 'live' reading).
 */
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

/**
 * @brief Represents the metdata associated with a set of Risk Score calculation values.
 * 
 */
struct RiskScoreMetadata {
  UUID agentId = UUID::unknown();
  UUID algorithmId = UUID::unknown();

  const bool operator==(const RiskScoreMetadata& other) const noexcept;
  const bool operator!=(const RiskScoreMetadata& other) const noexcept;
};

/**
 * @brief An array of Risk Scores associated to the same RiskScoreMetadata instance
 * 
 * @tparam ArraySize Maximum size of values against this RiskScoreMetadata instance. Defaults to one (i.e. the last ran analysis)
 */
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