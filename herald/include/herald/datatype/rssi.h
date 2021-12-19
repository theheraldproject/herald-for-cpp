//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_RSSI_H
#define HERALD_RSSI_H

#include "model.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class RSSI {
public:
  static constexpr ModelClass modelClassId{1};

  RSSI(); // default ctor (evaluates to 0)
  RSSI(int value); // int
  RSSI(const RSSI& other); // copy
  RSSI(RSSI&& other); // move
  ~RSSI();

  RSSI& operator=(const RSSI& other); // copy assign
  RSSI& operator=(RSSI&& other); // move assign
  
  bool operator==(const int other) const noexcept;
  bool operator!=(const int other) const noexcept;
  bool operator==(const RSSI& other) const noexcept;
  bool operator!=(const RSSI& other) const noexcept;
  bool operator<(const RSSI& other) const noexcept;
  bool operator<=(const RSSI& other) const noexcept;
  bool operator>(const RSSI& other) const noexcept;
  bool operator>=(const RSSI& other) const noexcept;

  operator long() const noexcept;
  operator double() const noexcept;

  std::size_t hashCode() const noexcept;

  operator std::string() const noexcept;

  int intValue() const noexcept;

private:
  int value;
};

} // end namespace
} // end namespace

#endif