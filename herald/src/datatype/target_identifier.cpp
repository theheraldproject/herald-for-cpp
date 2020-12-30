//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/target_identifier.h"

namespace herald {
namespace datatype {

class TargetIdentifier::Impl {
public:
  Impl();
  Impl(std::string v);
  ~Impl() = default;

  std::string value;
};

TargetIdentifier::Impl::Impl()
 : value("")
{
  ;
}

TargetIdentifier::Impl::Impl(std::string v)
 : value(std::move(v))
{
  ;
}







TargetIdentifier::TargetIdentifier()
 : mImpl(std::make_unique<Impl>())
{
  ; // TODO set value to random v4 UUID string
}

// PLATFORM SPECIFIC CTORS BEGIN

//TargetIdentifier::TargetIdentifier(BluetoothDevice device) {
//  // TODO do something specific to each platform here
//}
// PLATFORM SPECIFIC CTORS END

TargetIdentifier::~TargetIdentifier() {}

bool
TargetIdentifier::operator==(const TargetIdentifier& other) const noexcept {
  return hashCode() == other.hashCode();
}

bool
TargetIdentifier::operator==(const std::string& other) const noexcept {
  return hashCode() == std::hash<std::string>{}(other);
}

std::size_t
TargetIdentifier::hashCode() const {
  return std::hash<std::string>{}(mImpl->value);
}

std::string
TargetIdentifier::toString() const {
  return mImpl->value; // copy ctor
}


} // end namespace
} // end namespace
