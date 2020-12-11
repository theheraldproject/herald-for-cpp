//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "datatype/rssi.h"

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

RSSI::RSSI(RSSI&& other)
 : mImpl(std::make_unique<Impl>())
{
  mImpl->value = other.mImpl->value;
}

RSSI::~RSSI() {}

bool
RSSI::operator==(const RSSI& other) {
  return hashCode() == other.hashCode();
}

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

} // end namespace
} // end namespace
