//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/rssi.h"

#include <string>

namespace herald {
namespace datatype {

// class RSSI::Impl {
// public:
//   Impl();
//   ~Impl() = default;

//   int value;
// };

// RSSI::Impl::Impl() : value(0) { }

RSSI::RSSI()
 : value(0)
{
  ;
}

RSSI::RSSI(int value)
 : value(value)
{
  ;
}

RSSI::RSSI(const RSSI& other)
 : value(other.value)
{
  ;
}

RSSI::RSSI(RSSI&& other)
 : value(other.value)
{
  ;
}

RSSI::~RSSI() {}

RSSI&
RSSI::operator=(const RSSI& other)
{
  value = other.value;
  return *this;
}

RSSI&
RSSI::operator=(RSSI&& other)
{
  value = other.value;
  return *this;
}

std::size_t
RSSI::hashCode() const noexcept {
  return std::hash<int>{}(value);
}

RSSI::operator std::string() const noexcept {
  return "RSSI{value=" + std::to_string(value) + "}";
}

int
RSSI::intValue() const noexcept {
  return value;
}

RSSI::operator long() const noexcept {
  return value;
}

RSSI::operator double() const noexcept {
  return (double)value;
}



bool
RSSI::operator==(const int other) const noexcept
{
  return value == other;
}

bool
RSSI::operator!=(const int other) const noexcept
{
  return value != other;
}

bool
RSSI::operator==(const RSSI& other) const noexcept
{
  return value == other.value;
}

bool
RSSI::operator!=(const RSSI& other) const noexcept
{
  return value != other.value;
}

bool
RSSI::operator<(const RSSI& other) const noexcept
{
  return value < other.value;
}

bool
RSSI::operator<=(const RSSI& other) const noexcept
{
  return value <= other.value;
}

bool
RSSI::operator>(const RSSI& other) const noexcept
{
  return value > other.value;
}

bool
RSSI::operator>=(const RSSI& other) const noexcept
{
  return value >= other.value;
}





} // end namespace
} // end namespace
