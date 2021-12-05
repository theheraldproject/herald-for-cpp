//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/uuid.h"
#include "herald/datatype/data.h"
#include "herald/datatype/randomness.h"
#include "herald/datatype/base64_string.h"

#include <string>
#include <algorithm>
#include <array>
#include <sstream>
#include <iosfwd>
#include <iomanip>

#include <type_traits>

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
  Base64String asString;
  // remove hyphens before using hex decoding
  std::string newFrom = from; // copy
  newFrom.erase(std::remove(newFrom.begin(),newFrom.end(),'-'), newFrom.end());
  auto dataInstance = Data::fromHexEncodedString(newFrom);

  std::array<value_type, 16> data{ {0} };
  if (dataInstance.size() != 16) {
    return UUID(data,false);
  }
  for (std::size_t pos = 0;pos < 16;++pos) {
    data[pos] = (value_type)dataInstance.at(pos);
  }
  UUID uuid(data,true); // TODO check UUID is a V4 format
  return uuid;
}

// Instance functions
UUID::UUID(const char* from) noexcept
 : mData({0}), mValid(true)
{
  // TODO actually copy the value from the string
}

UUID::UUID(UUID&& from) noexcept
 : mData(std::move(from.mData)), mValid(from.mValid)
{
  ;
}

UUID::UUID(const UUID& from) noexcept
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
  // return mData == other.mData;
  bool eq = true;
  for (std::size_t i = 0;eq && i < max_size;++i) {
    eq = eq && (mData[i] == other.mData[i]);
  }
  return eq;
}
bool
UUID::operator!=(const UUID& other) const noexcept {
  return !(*this == other);
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

// Static assertions on this classes compiler contract
static_assert(std::is_constructible_v<UUID,const char*>,"UUID Cannot be string constructed");
static_assert(std::is_copy_constructible_v<UUID>,"UUID Cannot be copy constructed");
static_assert(std::is_move_constructible_v<UUID>,"UUID Cannot be move constructed");

} // end namespace
} // end namespace
