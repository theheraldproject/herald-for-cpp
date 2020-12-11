//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef UUID_H
#define UUID_H

#include "error_code.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

// High level UUID class template
class UUID {
public:
  using value_type = uint8_t;

  static UUID fromString(const std::string& from) noexcept;
  static UUID random() noexcept;

  UUID(UUID& from);
  ~UUID();

  bool valid() const noexcept;

  bool operator==(const UUID& other) const noexcept;
  bool operator!=(const UUID& other) const noexcept;
  bool operator<(const UUID& other) const noexcept;
  bool operator>(const UUID& other) const noexcept;
  std::string operator=(const UUID& from) const noexcept; // TODO verify this syntax/location

  std::array<value_type, 16> data() const noexcept;
  std::string string() const noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;

  UUID(std::array<value_type, 16> data, bool isValid) noexcept;
};



} // end namespace
} // end namespace

#endif