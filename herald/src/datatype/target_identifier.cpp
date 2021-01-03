//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/target_identifier.h"
#include "herald/datatype/data.h"

namespace herald {
namespace datatype {

class TargetIdentifier::Impl {
public:
  Impl();
  Impl(const Data& mac);
  Impl(const TargetIdentifier& from);
  ~Impl() = default;

  Data value;
};

TargetIdentifier::Impl::Impl()
 : value()
{
  ;
}

TargetIdentifier::Impl::Impl(const Data& v)
 : value(v)
{
  ;
}

TargetIdentifier::Impl::Impl(const TargetIdentifier& v)
 : value((Data)v) // conversion operator
{
  ;
}







TargetIdentifier::TargetIdentifier()
 : mImpl(std::make_unique<Impl>())
{
  ; // TODO set value to random v4 UUID string
}

TargetIdentifier::TargetIdentifier(const Data& data)
  : mImpl(std::make_unique<Impl>(data))
{
  ;
}

TargetIdentifier::TargetIdentifier(const TargetIdentifier& from)
  : mImpl(std::make_unique<Impl>(from))
{
  ;
}

TargetIdentifier::~TargetIdentifier() {}

bool
TargetIdentifier::operator==(const TargetIdentifier& other) const noexcept {
  return hashCode() == other.hashCode();
}

bool
TargetIdentifier::operator==(const Data& other) const noexcept {
  return mImpl->value == other;
}

bool
TargetIdentifier::operator!=(const TargetIdentifier& other) const noexcept {
  return hashCode() != other.hashCode();
}

bool
TargetIdentifier::operator!=(const Data& other) const noexcept {
  return mImpl->value != other;
}

std::size_t
TargetIdentifier::hashCode() const {
  return std::hash<Data>{}(mImpl->value);
}

TargetIdentifier::operator std::string() const {
  return mImpl->value.description();
}

TargetIdentifier::operator Data() const {
  return mImpl->value;
}

} // end namespace
} // end namespace
