//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/target_identifier.h"
#include "herald/datatype/data.h"

namespace herald {
namespace datatype {

// class TargetIdentifier::Impl {
// public:
//   Impl();
//   Impl(const Data& mac);
//   Impl(const TargetIdentifier& from);
//   ~Impl() = default;

//   Data value;
// };

// TargetIdentifier::Impl::Impl()
//  : value()
// {
//   ;
// }

// TargetIdentifier::Impl::Impl(const Data& v)
//  : value(v)
// {
//   ;
// }

// TargetIdentifier::Impl::Impl(const TargetIdentifier& v)
//  : value((Data)v) // conversion operator
// {
//   ;
// }







TargetIdentifier::TargetIdentifier()
 : value()
{
  ; // TODO set value to random v4 UUID string
}

TargetIdentifier::TargetIdentifier(const Data& data)
  : value(data)
{
  ;
}

TargetIdentifier::TargetIdentifier(const TargetIdentifier& from)
  : value(from.value)
{
  ;
}

TargetIdentifier::~TargetIdentifier() {}

TargetIdentifier&
TargetIdentifier::operator=(const TargetIdentifier& from)
{
  value = from.value;
  return *this;
}

bool
TargetIdentifier::operator==(const TargetIdentifier& other) const noexcept {
  return hashCode() == other.hashCode();
}

bool
TargetIdentifier::operator==(const Data& other) const noexcept {
  return value == other;
}

bool
TargetIdentifier::operator!=(const TargetIdentifier& other) const noexcept {
  return hashCode() != other.hashCode();
}

bool
TargetIdentifier::operator!=(const Data& other) const noexcept {
  return value != other;
}
bool
TargetIdentifier::operator<(const TargetIdentifier& other) const noexcept {
  return value < other.value;
}

bool
TargetIdentifier::operator>(const TargetIdentifier& other) const noexcept {
  return value > other.value;
}

std::size_t
TargetIdentifier::hashCode() const {
  return std::hash<Data>{}(value);
}

TargetIdentifier::operator std::string() const {
  return value.description();
}

TargetIdentifier::operator Data() const {
  return value;
}

Data
TargetIdentifier::underlyingData() const {
  return Data(value);
}

} // end namespace
} // end namespace
