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
  static UUID random(RandomnessGenerator& from) noexcept;

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