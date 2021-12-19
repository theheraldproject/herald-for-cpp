//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/luminosity.h"

#include <string>

namespace herald {
namespace datatype {

Luminosity::Luminosity()
 : value(0)
{
  ;
}

Luminosity::Luminosity(int value)
 : value(value)
{
  ;
}

Luminosity::Luminosity(const Luminosity& other)
 : value(other.value)
{
  ;
}

Luminosity::Luminosity(Luminosity&& other)
 : value(other.value)
{
  ;
}

Luminosity::~Luminosity() {}

Luminosity&
Luminosity::operator=(const Luminosity& other)
{
  value = other.value;
  return *this;
}

Luminosity&
Luminosity::operator=(Luminosity&& other)
{
  value = other.value;
  return *this;
}

std::size_t
Luminosity::hashCode() const noexcept {
  return std::hash<int>{}(value);
}

Luminosity::operator std::string() const noexcept {
  return "Luminosity{value=" + std::to_string(value) + "}";
}

int
Luminosity::intValue() const noexcept {
  return value;
}

Luminosity::operator long() const noexcept {
  return value;
}

Luminosity::operator double() const noexcept {
  return (double)value;
}



bool
Luminosity::operator==(const int other) const noexcept
{
  return value == other;
}

bool
Luminosity::operator!=(const int other) const noexcept
{
  return value != other;
}

bool
Luminosity::operator==(const Luminosity& other) const noexcept
{
  return value == other.value;
}

bool
Luminosity::operator!=(const Luminosity& other) const noexcept
{
  return value != other.value;
}

bool
Luminosity::operator<(const Luminosity& other) const noexcept
{
  return value < other.value;
}

bool
Luminosity::operator<=(const Luminosity& other) const noexcept
{
  return value <= other.value;
}

bool
Luminosity::operator>(const Luminosity& other) const noexcept
{
  return value > other.value;
}

bool
Luminosity::operator>=(const Luminosity& other) const noexcept
{
  return value >= other.value;
}





} // end namespace
} // end namespace
