//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_LUMINOSITY_H
#define HERALD_LUMINOSITY_H

#include "model.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class Luminosity {
public:
  static constexpr ModelClass modelClassId{3};

  Luminosity(); // default ctor (evaluates to 0)
  Luminosity(int value); // int
  Luminosity(const Luminosity& other); // copy
  Luminosity(Luminosity&& other); // move
  ~Luminosity();

  Luminosity& operator=(const Luminosity& other); // copy assign
  Luminosity& operator=(Luminosity&& other); // move assign
  
  bool operator==(const int other) const noexcept;
  bool operator!=(const int other) const noexcept;
  bool operator==(const Luminosity& other) const noexcept;
  bool operator!=(const Luminosity& other) const noexcept;
  bool operator<(const Luminosity& other) const noexcept;
  bool operator<=(const Luminosity& other) const noexcept;
  bool operator>(const Luminosity& other) const noexcept;
  bool operator>=(const Luminosity& other) const noexcept;

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