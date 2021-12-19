//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_MODEL_H
#define HERALD_MODEL_H

#include "uuid.h"

namespace herald {
namespace datatype {

/** /brief Specific strongly types override of UUID for Model Class Ids (used in Analysis and Exposure API, but not dependent upon them) **/
struct ModelClass : public UUID {
public:
  constexpr ModelClass(UUID::value_type shortCode) noexcept
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

    mData[max_size - 1] = value_type{3}; // explicit construction

    mValid = true;
  }

  ~ModelClass() noexcept = default;
};

}
}

#endif