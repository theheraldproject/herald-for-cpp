//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DISTANCE_H
#define HERALD_DISTANCE_H

#include <cstddef>

namespace herald {
namespace datatype {

/// Simple distance value in metres
/// Has to be a class/struct type to allow template resolution
struct Distance {
  double value;
  
  Distance(); // default ctor (evaluates to 0)
  Distance(double value);
  Distance(const Distance& other); // copy ctor
  Distance(Distance&& other); // move ctor
  ~Distance();

  Distance& operator=(const Distance& other); // copy assign
  Distance& operator=(Distance&& other); // move assign
  
  bool operator==(const double other) const noexcept;
  bool operator!=(const double other) const noexcept;
  bool operator==(const Distance& other) const noexcept;
  bool operator!=(const Distance& other) const noexcept;
  bool operator<(const Distance& other) const noexcept;
  bool operator<=(const Distance& other) const noexcept;
  bool operator>(const Distance& other) const noexcept;
  bool operator>=(const Distance& other) const noexcept;

  operator double() const noexcept;

  std::size_t hashCode() const noexcept;
};

}
}

#endif