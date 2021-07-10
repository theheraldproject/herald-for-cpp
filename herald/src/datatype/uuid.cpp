//  Copyright 2020-2021 Herald Project Contributors
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
// class UUID::Impl {
// public:
//   using value_type = uint8_t;

//   Impl(std::array<value_type, 16>& data, bool isValid);
//   ~Impl();

//   std::array<value_type, 16> mData = { {0}};
//   bool mValid;
// };

// UUID::Impl::Impl(std::array<value_type, 16>& data, bool isValid)
//  : mValid(isValid)
// {
//   mData = std::move(data);
// }

// UUID::Impl::~Impl() {
//   ;
// }



// Implementation Class

// Static functions
UUID
UUID::fromString(const std::string& from) noexcept {
  std::array<value_type, 16> data{ {0} };
  UUID uuid(data,false); // TODO parse string, determine if valid, and tag as v4
  return uuid; // returns copy
}

// Instance functions

UUID::UUID(UUID&& from) 
 : mData(std::move(from.mData)), mValid(from.mValid)
{
  ;
}

UUID::UUID(const UUID& from) 
 : mData(from.mData),mValid(from.mValid)
{
  ;
}

// private ctor
UUID::UUID(std::array<value_type, 16> data, bool isValid) noexcept
 : mData(data),mValid(isValid)
{
  ;
}

UUID::~UUID() = default;


UUID&
UUID::operator=(const UUID& other) noexcept
{
  mData = other.mData;
  mValid = other.mValid;
  return *this;
}

bool
UUID::valid() const noexcept {
  return mValid;
}

bool
UUID::operator==(const UUID& other) const noexcept {
  return mData == other.mData;
}
bool
UUID::operator!=(const UUID& other) const noexcept {
  return mData != other.mData;
}

bool
UUID::operator<(const UUID& other) const noexcept {
  return mData < other.mData;
}

bool
UUID::operator>(const UUID& other) const noexcept {
  return mData > other.mData;
}

std::array<uint8_t, 16>
UUID::data() const noexcept {
  return mData;
}

std::string
UUID::string() const noexcept {
  // convert bytes to hex string
  std::stringstream str;
	str.setf(std::ios_base::hex, std::ios::basefield);
	str.fill('0');
  for (std::size_t i=0; i < 16; i++) {
		str << std::setw(2) << (unsigned short)mData[i];
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
