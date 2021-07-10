//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_UUID_H
#define HERALD_UUID_H

#include "error_code.h"
#include "randomness.h"

#include <string>
#include <array>

namespace herald {
namespace datatype {

// High level UUID class template
class UUID {
public:
  using value_type = uint8_t;

  static UUID fromString(const std::string& from) noexcept;

  template <typename RandomnessSourceT>
  static UUID random(RandomnessGenerator<RandomnessSourceT>& from) noexcept {
    std::array<value_type, 16> data{ {0} };
    Data randomness;
    from.nextBytes(16,randomness);
    for (std::size_t i = 0;i < 16;i++) {
      data[i] = (value_type)randomness.at(i);
    }
    // Now set bits for v4 UUID explicitly
    constexpr value_type M = 0x40; // 7th byte = 0100 in binary for MSB 0000 for LSB - v4 UUID
    constexpr value_type N = 0x80; // 9th byte = 1000 in binary for MSB 0000 for LSB - variant 1
    data[6] = (0x0f & data[6]) | M; // blanks out first 4 bits
    data[8] = (0x3f & data[8]) | N; // blanks out first 2 bits
    UUID uuid(data,false); // TODO generate random data and tag as v4
    return uuid; // returns copy
  }

  UUID(UUID&& from);
  UUID(const UUID& from);
  ~UUID();

  UUID& operator=(const UUID& other) noexcept; // copy assign

  bool valid() const noexcept;

  bool operator==(const UUID& other) const noexcept;
  bool operator!=(const UUID& other) const noexcept;
  bool operator<(const UUID& other) const noexcept;
  bool operator<=(const UUID& other) const noexcept;
  bool operator>(const UUID& other) const noexcept;
  bool operator>=(const UUID& other) const noexcept;
  // std::string operator=(const UUID& from) const noexcept; // TODO verify this syntax/location

  std::array<value_type, 16> data() const noexcept;
  std::string string() const noexcept;

private:
  std::array<value_type, 16> mData = { {0}};
  bool mValid;

  UUID(std::array<value_type, 16> data, bool isValid) noexcept;
};



} // end namespace
} // end namespace

#endif