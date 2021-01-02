//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/data.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace herald {
namespace datatype {

// DATA PIMPL DEFINITIONS

class Data::Impl {
public:
  Impl();
  ~Impl() = default;

  std::vector<std::byte> data;
};

// DATA PIMPL DECLARATIONS
Data::Impl::Impl() : data() { }





// DATA CLASS DECLARATIONS
const char Data::hexChars[] = {
  '0','1','2','3','4','5','6','7',
  '8','9','a','b','c','d','e','f'
};

Data::Data() : mImpl(std::make_unique<Impl>()) {
  ;
}

Data::Data(Data&& other)
  : mImpl(std::move(other.mImpl))
{
  ;
}

Data::Data(const std::byte* value, std::size_t length) : mImpl(std::make_unique<Impl>()) {
  mImpl->data.reserve(length);
  for (std::size_t i = 0;i < length; i++) {
    mImpl->data.push_back(value[i]);
  }
}

Data::Data(const std::uint8_t* value, std::size_t length) : 
  mImpl(std::make_unique<Impl>()) {
  
  mImpl->data.reserve(length);
  for (std::size_t i = 0;i < length; i++) {
    mImpl->data.push_back(std::byte(value[i]));
  }
}

Data::Data(std::vector<std::byte> value) : mImpl(std::make_unique<Impl>()) {
  mImpl->data = std::move(value);
}

Data::Data(const Data& from) : mImpl(std::make_unique<Impl>()) {
  mImpl->data = from.mImpl->data; // copy ctor
}

Data::Data(std::byte repeating, std::size_t count) : mImpl(std::make_unique<Impl>()) {
  mImpl->data.reserve(count);
  for (std::size_t i = 0;i < count; i++) {
    mImpl->data.push_back(repeating);
  }
}

Data&
Data::operator=(const Data& other)
{
  mImpl->data = other.mImpl->data;
  return *this;
}

Data::~Data() {}

Data
Data::fromHexEncodedString(const std::string& hex)
{
  Data d;
  // parse string
  const std::size_t length = hex.size();
  std::string hexInput;
  // Input size check - two characters per single byte
  if (1 == length % 2) {
    // invalid format - not an even number of characters
    // Prepend input with a 0. (Note '8' and '08' in hex are the same)
    hexInput += "0";
  }
  hexInput += hex;
  for (std::size_t i = 0; i < length; i += 2) {
    d.append((std::uint8_t)( // forcing for the compiler to a single std::uint8_t
       std::uint8_t(hexChars[std::size_t(hex[i    ] % 16)] << 4)   | 
      (std::uint8_t(hexChars[std::size_t(hex[i + 1] % 16)] & 0x0f)
    )));
  }
  return d;
}

std::string
Data::hexEncodedString() const
{
  if (0 == mImpl->data.size()) {
    return "";
  }
  std::string result;
  std::size_t size = mImpl->data.size();
  result.reserve(size * 2);
  std::size_t v;
  for (std::size_t i = 0; i < size; i++) {
    v = std::size_t(mImpl->data.at(i));
    result += hexChars[0x0F & (v >> 4)]; // MSB
    result += hexChars[0x0F &  v      ]; // LSB
  }
  return result;
}

std::string
Data::description() const {
  return hexEncodedString();
}

Data
Data::subdata(std::size_t offset) const {
  Data copy;
  if (offset >= mImpl->data.size()) {
    return std::move(copy);
  }
  std::copy(mImpl->data.begin() + offset, mImpl->data.end(), std::back_inserter(copy.mImpl->data));
  return std::move(copy);
}

Data
Data::subdata(std::size_t offset, std::size_t length) const {
  Data copy;
  // Note: offset if passed as -1 could be MAX_LONG_LONG, so check it on its own too
  if (offset >= mImpl->data.size() || offset + length > mImpl->data.size()) {
    return std::move(copy);
  }
  std::copy(mImpl->data.begin() + offset, mImpl->data.begin() + offset + length, std::back_inserter(copy.mImpl->data));
  return std::move(copy);
}

std::byte
Data::at(std::size_t index) const {
  if (index > mImpl->data.size() - 1) {
    return std::byte(0);
  }
  return mImpl->data[index];
}

bool
Data::uint8(std::size_t fromIndex, uint8_t& into) const noexcept
{
  if (fromIndex > mImpl->data.size() - 1) {
    return false;
  }
  into = std::uint8_t(mImpl->data[fromIndex]);
  return true;
}

bool
Data::uint16(std::size_t fromIndex, uint16_t& into) const noexcept
{
  if (fromIndex > mImpl->data.size() - 2) {
    return false;
  }
  into = (std::uint16_t(mImpl->data[fromIndex + 1]) << 8) | std::uint16_t(mImpl->data[fromIndex]);
  return true;
}

bool
Data::uint32(std::size_t fromIndex, uint32_t& into) const noexcept
{
  if (fromIndex > mImpl->data.size() - 4) {
    return false;
  }
  into =  std::uint32_t(mImpl->data[fromIndex])            | (std::uint32_t(mImpl->data[fromIndex + 1]) << 8) |
         (std::uint32_t(mImpl->data[fromIndex + 2]) << 16) | (std::uint32_t(mImpl->data[fromIndex + 3]) << 24);
  return true;
}

bool
Data::uint64(std::size_t fromIndex, uint64_t& into) const noexcept
{
  if (fromIndex > mImpl->data.size() - 8) {
    return false;
  }
  into = (std::uint64_t(mImpl->data[fromIndex + 7]) << 56) | (std::uint64_t(mImpl->data[fromIndex + 6]) << 48) |
         (std::uint64_t(mImpl->data[fromIndex + 5]) << 40) | (std::uint64_t(mImpl->data[fromIndex + 4]) << 32) |
         (std::uint64_t(mImpl->data[fromIndex + 3]) << 24) | (std::uint64_t(mImpl->data[fromIndex + 2]) << 16) |
         (std::uint64_t(mImpl->data[fromIndex + 1]) << 8)  |  std::uint64_t(mImpl->data[fromIndex]);
  return true;
}

void
Data::append(const Data& data, std::size_t offset, std::size_t length)
{
  mImpl->data.reserve(mImpl->data.size() + length);
  std::copy(data.mImpl->data.begin() + offset, 
            data.mImpl->data.begin() + offset + length, 
            std::back_inserter(mImpl->data)
  );
}

void
Data::appendReversed(const Data& data, std::size_t offset, std::size_t length)
{
  mImpl->data.reserve(mImpl->data.size() + length);
  std::reverse_copy(data.mImpl->data.begin() + offset, 
                    data.mImpl->data.begin() + offset + length, 
                    std::back_inserter(mImpl->data)
  );
}

void
Data::append(const Data& data) {
  mImpl->data.reserve(mImpl->data.size() + data.size());
  std::copy(data.mImpl->data.begin(), data.mImpl->data.end(), std::back_inserter(mImpl->data));
}

void
Data::append(uint8_t data)
{
  std::size_t curSize = mImpl->data.size();
  mImpl->data.reserve(curSize + 1); // C++ ensures types are AT LEAST x bits
  mImpl->data.push_back(std::byte(data));
  curSize++;
}

void
Data::append(uint16_t data)
{
  std::size_t curSize = mImpl->data.size();
  mImpl->data.reserve(curSize + 2); // C++ ensures types are AT LEAST x bits
  mImpl->data.push_back(std::byte(data & 0xff));
  mImpl->data.push_back(std::byte(data >> 8));
}

void
Data::append(uint32_t data)
{
  std::size_t curSize = mImpl->data.size();
  mImpl->data.reserve(curSize + 4); // C++ ensures types are AT LEAST x bits
  mImpl->data.push_back(std::byte(data & 0xff));
  mImpl->data.push_back(std::byte(data >> 8));
  mImpl->data.push_back(std::byte(data >> 16));
  mImpl->data.push_back(std::byte(data >> 24));
}

void
Data::append(uint64_t data)
{
  std::size_t curSize = mImpl->data.size();
  mImpl->data.reserve(curSize + 8); // C++ ensures types are AT LEAST x bits
  mImpl->data.push_back(std::byte(data & 0xff));
  mImpl->data.push_back(std::byte(data >> 8));
  mImpl->data.push_back(std::byte(data >> 16));
  mImpl->data.push_back(std::byte(data >> 24));
  mImpl->data.push_back(std::byte(data >> 32));
  mImpl->data.push_back(std::byte(data >> 40));
  mImpl->data.push_back(std::byte(data >> 48));
  mImpl->data.push_back(std::byte(data >> 56));
}

void
Data::append(const std::string& data)
{
  std::size_t curSize = mImpl->data.size();
  mImpl->data.reserve(curSize + data.size());
  for (std::size_t i = 0;i < data.size();i++) {
    mImpl->data.push_back(std::byte(data[i]));
  }
}

bool
Data::operator==(const Data& other) const noexcept {
  if (size() != other.size()) {
    return false;
  }
  //if (hashCode() != other.hashCode()) {
  //  return false;
  //}
  // else compare each value

  // alternatively, cheat...
  return hashCode() == other.hashCode(); // Somewhat naughty
}


std::size_t
Data::hashCode() const {
  // TODO consider a faster (E.g. SIMD) algorithm or one with less hotspots (see hashdos attacks)
  return std::hash<std::vector<std::byte>>{}(mImpl->data);
}

std::size_t
Data::size() const {
  return mImpl->data.size();
}

} // end namespace
} // end namespace