//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/distance.h"

#include <functional>

namespace herald {
namespace datatype {

Distance::Distance() : value(0.0)
{
  ;
}

Distance::Distance(double value) : value(value)
{
  ;
}

Distance::Distance(const Distance& other)
 : value(other.value)
{
  ;
}

Distance::Distance(Distance&& other)
 : value(other.value)
{
  ;
}

Distance::~Distance() = default;

Distance&
Distance::operator=(const Distance& other)
{
  value = other.value;
  return *this;
}

Distance&
Distance::operator=(Distance&& other)
{
  value = other.value;
  return *this;
}

std::size_t
Distance::hashCode() const noexcept {
  return std::hash<double>{}(value);
}

Distance::operator double() const noexcept {
  return value;
}



bool
Distance::operator==(const double other) const noexcept
{
  return value == other;
}

bool
Distance::operator!=(const double other) const noexcept
{
  return value != other;
}

bool
Distance::operator==(const Distance& other) const noexcept
{
  return value == other.value;
}

bool
Distance::operator!=(const Distance& other) const noexcept
{
  return value != other.value;
}

bool
Distance::operator<(const Distance& other) const noexcept
{
  return value < other.value;
}

bool
Distance::operator<=(const Distance& other) const noexcept
{
  return value <= other.value;
}

bool
Distance::operator>(const Distance& other) const noexcept
{
  return value > other.value;
}

bool
Distance::operator>=(const Distance& other) const noexcept
{
  return value >= other.value;
}





} // end namespace
} // end namespace
