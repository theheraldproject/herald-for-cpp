//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/rssi.h"

#include <string>

namespace herald {
namespace datatype {

class RSSI::Impl {
public:
  Impl();
  ~Impl() = default;

  int value;
};

RSSI::Impl::Impl() : value(0) { }


RSSI::RSSI(int value)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->value = value;
}

RSSI::RSSI(const RSSI& other)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->value = other.mImpl->value;
}

RSSI::RSSI(RSSI&& other)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->value = other.mImpl->value;
}

RSSI::~RSSI() {}

RSSI&
RSSI::operator=(const RSSI& other)
{
  mImpl->value = other.mImpl->value;
  return *this;
}

RSSI&
RSSI::operator=(RSSI&& other)
{
  mImpl->value = other.mImpl->value;
  return *this;
}

std::size_t
RSSI::hashCode() const noexcept {
  return std::hash<int>{}(mImpl->value);
}

RSSI::operator std::string() const noexcept {
  return "RSSI{value=" + std::to_string(mImpl->value) + "}";
}

int
RSSI::intValue() const noexcept {
  return mImpl->value;
}




bool
RSSI::operator==(const RSSI& other) const noexcept
{
  return mImpl->value == other.mImpl->value;
}

bool
RSSI::operator!=(const RSSI& other) const noexcept
{
  return mImpl->value != other.mImpl->value;
}

bool
RSSI::operator<(const RSSI& other) const noexcept
{
  return mImpl->value < other.mImpl->value;
}

bool
RSSI::operator<=(const RSSI& other) const noexcept
{
  return mImpl->value <= other.mImpl->value;
}

bool
RSSI::operator>(const RSSI& other) const noexcept
{
  return mImpl->value > other.mImpl->value;
}

bool
RSSI::operator>=(const RSSI& other) const noexcept
{
  return mImpl->value >= other.mImpl->value;
}





} // end namespace
} // end namespace
