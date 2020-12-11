//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "datatype/uuid.h"

#include <string>
#include <array>

namespace herald {
namespace datatype {

// PIMPL Class
class UUID::Impl {
public:
  using value_type = uint8_t;

  Impl(std::array<value_type, 16>& data, bool isValid);
  ~Impl();

  std::array<value_type, 16> mData = { {0}};
  bool mValid;
};

UUID::Impl::Impl(std::array<value_type, 16>& data, bool isValid)
 : mValid(isValid)
{
  mData = std::move(data);
}

UUID::Impl::~Impl() {
  ;
}




// Implementation Class

// Static functions
UUID
UUID::fromString(const std::string& from) noexcept {
  std::array<value_type, 16> data{ {0} };
  UUID uuid(data,false); // TODO parse string, determine if valid, and tag as v4
  return std::move(uuid);
}

UUID
UUID::random() noexcept {
  std::array<value_type, 16> data{ {0} };
  UUID uuid(data,false); // TODO generate random data and tag as v4
  return std::move(uuid);
}

// Instance functions

UUID::UUID(UUID& from) 
 : mImpl(std::make_unique<Impl>(from.mImpl->mData,from.mImpl->mValid))
{
  ;
}

// private ctor
UUID::UUID(std::array<value_type, 16> data, bool isValid) noexcept
 : mImpl(std::make_unique<Impl>(data,isValid))
{
  ;
}

UUID::~UUID() {
  ;
}

bool
UUID::valid() const noexcept {
  return mImpl->mValid;
}

bool
UUID::operator==(const UUID& other) const noexcept {
  return mImpl->mData == other.mImpl->mData;
}
bool
UUID::operator!=(const UUID& other) const noexcept {
  return mImpl->mData != other.mImpl->mData;
}

bool
UUID::operator<(const UUID& other) const noexcept {
  return mImpl->mData < other.mImpl->mData;
}

bool
UUID::operator>(const UUID& other) const noexcept {
  return mImpl->mData > other.mImpl->mData;
}

std::string
UUID::operator=(const UUID& from) const noexcept {
  return "";
}

std::array<uint8_t, 16>
UUID::data() const noexcept {
  return mImpl->mData;
}

std::string
UUID::string() const noexcept {
  return ""; // TODO actual convert to a string
}

} // end namespace
} // end namespace
