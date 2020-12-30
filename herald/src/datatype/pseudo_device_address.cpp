//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/pseudo_device_address.h"
#include "herald/datatype/data.h"
#include "herald/datatype/base64_string.h"

#include <vector>
#include <string>

namespace herald {
namespace datatype {

// PIMPL DEFINITION
class PseudoDeviceAddress::Impl {
public:
  Impl();
  ~Impl() = default;

  std::vector<std::byte> data;
  long address;
};

// PIMPL DECLARATIONS
PseudoDeviceAddress::Impl::Impl()
 : data(), address(0)
{
  ;
}


// CLASS DECLARATIONS
PseudoDeviceAddress::PseudoDeviceAddress()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

PseudoDeviceAddress::PseudoDeviceAddress(const std::byte* data, const std::size_t length)
  : mImpl(std::make_unique<Impl>())
{
  mImpl->data.reserve(length);
  for (std::size_t i = 0;i < length;i++) {
    mImpl->data.push_back(data[i]);
  }
}

PseudoDeviceAddress::~PseudoDeviceAddress() {}

bool
PseudoDeviceAddress::operator==(const PseudoDeviceAddress& other) const noexcept {
  return hashCode() == other.hashCode();
}

std::size_t
PseudoDeviceAddress::hashCode() const {
  return std::hash<std::vector<std::byte>>{}(mImpl->data);
}

std::string
PseudoDeviceAddress::toString() const {
  Data d(mImpl->data);
  Base64String b64s = Base64String::encode(d);
  return b64s.encoded(); // returns a copy and so is safe
}


} // end namespace
} // end namespace
