//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_RSSI_MINUTE_H
#define HERALD_RSSI_MINUTE_H

#include "model.h"

#include <string>

namespace herald {
namespace datatype {

class RSSIMinute {
public:
  static constexpr ModelClass modelClassId{42};
  
  RSSIMinute(); // default ctor (evaluates to 0)
  RSSIMinute(double value); // double
  RSSIMinute(const RSSIMinute& other); // copy
  RSSIMinute(RSSIMinute&& other); // move
  ~RSSIMinute();

  RSSIMinute& operator=(const RSSIMinute& other); // copy assign
  RSSIMinute& operator=(RSSIMinute&& other); // move assign
  
  bool operator==(const double other) const noexcept;
  bool operator!=(const double other) const noexcept;
  bool operator==(const RSSIMinute& other) const noexcept;
  bool operator!=(const RSSIMinute& other) const noexcept;
  bool operator<(const RSSIMinute& other) const noexcept;
  bool operator<=(const RSSIMinute& other) const noexcept;
  bool operator>(const RSSIMinute& other) const noexcept;
  bool operator>=(const RSSIMinute& other) const noexcept;

  operator double() const noexcept;

  std::size_t hashCode() const noexcept;

  operator std::string() const noexcept;

  double doubleValue() const noexcept;

private:
  double value;
};

} // end namespace
} // end namespace

#endif