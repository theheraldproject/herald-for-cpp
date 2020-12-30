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

std::size_t
RSSI::hashCode() const {
  return std::hash<int>{}(mImpl->value);
}

std::string
RSSI::toString() const {
  return "RSSI{value=" + std::to_string(mImpl->value) + "}";
}

int
RSSI::intValue() const {
  return mImpl->value;
}







bool
operator==(const RSSI& first, const RSSI& other) {
  return first.hashCode() == other.hashCode();
}



} // end namespace
} // end namespace
