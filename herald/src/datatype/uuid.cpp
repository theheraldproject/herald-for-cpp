//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/uuid.h"
#include "herald/datatype/data.h"
#include "herald/datatype/randomness.h"

#include <string>
#include <array>
#include <sstream>
#include <iosfwd>
#include <iomanip>

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
  return uuid; // returns copy
}

UUID
UUID::random(RandomnessGenerator& from) noexcept {
  std::array<value_type, 16> data{ {0} };
  Data randomness;
  from.nextBytes(16,randomness);
  for (std::size_t i = 0;i < 16;i++) {
    data[i] = (value_type)randomness.at(i);
  }
  // Now set bits for v4 UUID explicitly
  constexpr value_type M = 0x40; // 7th byte = 0100 in binary for MSB 0000 for LSB - v4 UUID
  constexpr value_type N = 0x80; // 9th byte = 1000 in binary for MSB 0000 for LSB - variant 1
  data[6] = (0x0f & data[6]) | M; // blanks out first 4 bits
  data[8] = (0x3f & data[8]) | N; // blanks out first 2 bits
  UUID uuid(data,false); // TODO generate random data and tag as v4
  return uuid; // returns copy
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
  return from.string();
}

std::array<uint8_t, 16>
UUID::data() const noexcept {
  return mImpl->mData;
}

std::string
UUID::string() const noexcept {
  // convert bytes to hex string
  std::stringstream str;
	str.setf(std::ios_base::hex, std::ios::basefield);
	str.fill('0');
  for (std::size_t i=0; i < 16; i++) {
		str << std::setw(2) << (unsigned short)mImpl->mData[i];
	}
	std::string hexString = str.str();
  // add in hyphens at relevant points
  std::stringstream fstr;
  fstr << hexString.substr(0,8) << "-" << hexString.substr(8,4) << "-"
       << hexString.substr(12,4) << "-" << hexString.substr(16,4) << "-"
       << hexString.substr(20,12);
  return fstr.str();
}

} // end namespace
} // end namespace
