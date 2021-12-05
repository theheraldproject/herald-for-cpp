//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_PARAMETERS_H
#define HERALD_EXPOSURE_PARAMETERS_H

#include "../datatype/uuid.h"
#include "../datatype/allocatable_array.h"

namespace herald {
namespace exposure {

/** /brief Specific strongly types override of UUID for Sensor Calsses **/
struct RiskParameter : public herald::datatype::UUID {
public:
  constexpr RiskParameter() noexcept
   : herald::datatype::UUID(std::array<value_type, max_size>{0})
  {
    // leave as default (empty)
  }

  constexpr RiskParameter(herald::datatype::UUID::value_type shortCode) noexcept
   : herald::datatype::UUID(std::array<value_type, max_size>{0})
  {
    // Copy short code to first position
    mData[0] = shortCode;
    // Copy agent flags into last position
    // Copy V4 valid UUID into other positions
    constexpr value_type M = 0x40; // 7th byte = 0100 in binary for MSB 0000 for LSB - v4 UUID
    constexpr value_type N = 0x80; // 9th byte = 1000 in binary for MSB 0000 for LSB - variant 1
    mData[6] = (0x0f & mData[6]) | M; // blanks out first 4 bits
    mData[8] = (0x3f & mData[8]) | N; // blanks out first 2 bits

    mData[max_size - 1] = value_type{3}; // explicit construction

    mValid = true;
  }

  ~RiskParameter() noexcept = default;
};

/**
 * @brief A collection of arbitrary Parameters.
 */
template <std::size_t MaxParameters = 8>
using RiskParameters = ArrayMap<RiskParameter, double, MaxParameters>;

/**
 * @brief Parameters that are typically more static and not derived from exposure. Typically metadata of a person's current condition.
 */
namespace parameter {

static constexpr RiskParameter weight{1};
static constexpr RiskParameter phenotypic_sex{2};
static constexpr RiskParameter age{3};

} // end parameter namespace



}
}

#endif