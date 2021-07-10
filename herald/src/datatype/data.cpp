//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/data.h"

#include <algorithm>
#include <iterator>
#include <vector>
#include <cstdlib>

namespace herald {
namespace datatype {

// DATA CLASS DECLARATIONS
const char Data::hexChars[] = {
  '0','1','2','3','4','5','6','7',
  '8','9','a','b','c','d','e','f'
};

Data::Data() : data()
{
  ;
}

Data::Data(Data&& other)
  : data()
{
  std::swap(data,other.data);
}

// TODO consider adding offset versions of the below two constructors

Data::Data(const std::byte* value, std::size_t length) : data(length) {
  for (std::size_t i = 0;i < length; i++) {
    data[i] = value[i];
  }
}

Data::Data(const std::uint8_t* value, std::size_t length) : 
  data(length) {
  
  for (std::size_t i = 0;i < length; i++) {
    data[i] = std::byte(value[i]);
  }
}

Data::Data(std::vector<std::byte> value) : data() {
  data = std::move(value);
}

Data::Data(const Data& from) : data() {
  data = from.data; // copy ctor
}

Data::Data(std::byte repeating, std::size_t count) : data(count) {
  for (std::size_t i = 0;i < count; i++) {
    data[i] = repeating;
  }
}

Data::Data(std::size_t reserveLength) : data(reserveLength) {
  ;
}

Data&
Data::operator=(const Data& other)
{
  data = other.data;
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

  for (std::size_t i = 0; i < hexInput.size(); i += 2) {
    std::string byteString = hexInput.substr(i, 2);
    std::byte byte = std::byte(strtol(byteString.c_str(), NULL, 16));
    d.data.push_back(byte);
  }

  return d;
}

std::string
Data::hexEncodedString() const noexcept
{
  if (0 == data.size()) {
    return "";
  }
  std::string result;
  std::size_t size = data.size();
  result.reserve(size * 2);
  std::size_t v;
  for (std::size_t i = 0; i < size; i++) {
    v = std::size_t(data.at(i));
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
  if (offset >= data.size()) {
    return copy;
  }
  std::copy(data.begin() + offset, data.end(), std::back_inserter(copy.data));
  return copy;
}

Data
Data::subdata(std::size_t offset, std::size_t length) const {
  Data copy;
  // Note: offset if passed as -1 could be MAX_LONG_LONG, so check it on its own too
  if (offset >= data.size()) {
    return copy;
  }
  // Note the below is necessary as calling with (4,-1), the second condition IS valid!
  if (length > data.size() || offset + length > data.size()) {
    std::copy(data.begin() + offset, data.end(), std::back_inserter(copy.data));
  } else {
    std::copy(data.begin() + offset, data.begin() + offset + length, std::back_inserter(copy.data));
  }
  return copy;
}

std::byte
Data::at(std::size_t index) const {
  if (index > (data.size() - 1)) {
    return std::byte(0);
  }
  return data[index];
}

bool
Data::uint8(std::size_t fromIndex, uint8_t& into) const noexcept
{
  if (fromIndex > data.size() - 1) {
    return false;
  }
  into = std::uint8_t(data[fromIndex]);
  return true;
}

bool
Data::uint16(std::size_t fromIndex, uint16_t& into) const noexcept
{
  if (fromIndex > data.size() - 2) {
    return false;
  }
  into = (std::uint16_t(data[fromIndex + 1]) << 8) | std::uint16_t(data[fromIndex]);
  return true;
}

bool
Data::uint32(std::size_t fromIndex, uint32_t& into) const noexcept
{
  if (fromIndex > data.size() - 4) {
    return false;
  }
  into =  std::uint32_t(data[fromIndex])            | (std::uint32_t(data[fromIndex + 1]) << 8) |
         (std::uint32_t(data[fromIndex + 2]) << 16) | (std::uint32_t(data[fromIndex + 3]) << 24);
  return true;
}

bool
Data::uint64(std::size_t fromIndex, uint64_t& into) const noexcept
{
  if (fromIndex > data.size() - 8) {
    return false;
  }
  into = (std::uint64_t(data[fromIndex + 7]) << 56) | (std::uint64_t(data[fromIndex + 6]) << 48) |
         (std::uint64_t(data[fromIndex + 5]) << 40) | (std::uint64_t(data[fromIndex + 4]) << 32) |
         (std::uint64_t(data[fromIndex + 3]) << 24) | (std::uint64_t(data[fromIndex + 2]) << 16) |
         (std::uint64_t(data[fromIndex + 1]) << 8)  |  std::uint64_t(data[fromIndex]);
  return true;
}

void
Data::append(const std::uint8_t* rawData, std::size_t offset, std::size_t length)
{
  data.reserve(length);
  for (std::size_t i = 0;i < length;i++) {
    data.push_back(std::byte(rawData[offset + i]));
  }
}

void
Data::append(const Data& rawData, std::size_t offset, std::size_t length)
{
  data.reserve(rawData.size() + length);
  std::copy(rawData.data.begin() + offset, 
            rawData.data.begin() + offset + length, 
            std::back_inserter(data)
  );
}

void
Data::appendReversed(const Data& rawData, std::size_t offset, std::size_t length)
{
  if (offset > rawData.size()) {
    return; // append nothing - out of range
  }
  std::size_t checkedLength = length;
  if (length > (rawData.size() - offset)) {
    checkedLength = rawData.size() - offset;
  }
  data.reserve(rawData.size() + checkedLength);
  std::reverse_copy(rawData.data.begin() + offset, 
                    rawData.data.begin() + offset + checkedLength, 
                    std::back_inserter(data)
  );
}

Data
Data::reversed() const
{
  Data result;
  result.data.reserve(data.size());
  std::reverse_copy(data.begin(),data.end(),
    std::back_inserter(result.data)
  );
  return result;
}

Data
Data::reverseEndianness() const
{
  Data result;
  result.data.reserve(data.size());

  // Keep byte order intact (caller could use reversed() to change that)
  // but reverse the order of the individual bits by each byte
  std::uint8_t value, original;
  for (std::size_t i = 0;i < data.size();++i) {
    original = std::uint8_t(data[i]);
    value = 0;
    for (int b = 0;b < 8;++b) {
      if ((original & (1 << b)) > 0) {
        value |= 1 << (7 - b);
      }
    }
    // result.data[i] = std::byte(value);
    result.append(value);
  }

  return result;
}

void
Data::assign(const Data& other) {
  if (other.size() > data.size()) {
    data.reserve(other.size());
  }
  for (std::size_t pos = 0; pos < other.size();++pos) {
    data[pos] = other.data[pos];
  }
}

void
Data::append(const Data& rawData) {
  data.reserve(rawData.size() + data.size());
  std::copy(rawData.data.begin(), rawData.data.end(), std::back_inserter(data));
}

void
Data::append(uint8_t rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + 1); // C++ ensures types are AT LEAST x bits
  data.push_back(std::byte(rawData));
  curSize++;
}

void
Data::append(std::byte rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + 1);
  data.push_back(rawData);
  curSize++;
}

void
Data::append(uint16_t rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + 2); // C++ ensures types are AT LEAST x bits
  data.push_back(std::byte(rawData & 0xff));
  data.push_back(std::byte(rawData >> 8));
}

void
Data::append(uint32_t rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + 4); // C++ ensures types are AT LEAST x bits
  data.push_back(std::byte(rawData & 0xff));
  data.push_back(std::byte(rawData >> 8));
  data.push_back(std::byte(rawData >> 16));
  data.push_back(std::byte(rawData >> 24));
}

void
Data::append(uint64_t rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + 8); // C++ ensures types are AT LEAST x bits
  data.push_back(std::byte(rawData & 0xff));
  data.push_back(std::byte(rawData >> 8));
  data.push_back(std::byte(rawData >> 16));
  data.push_back(std::byte(rawData >> 24));
  data.push_back(std::byte(rawData >> 32));
  data.push_back(std::byte(rawData >> 40));
  data.push_back(std::byte(rawData >> 48));
  data.push_back(std::byte(rawData >> 56));
}

void
Data::append(const std::string& rawData)
{
  std::size_t curSize = data.size();
  data.reserve(curSize + rawData.size());
  for (std::size_t i = 0;i < rawData.size();i++) {
    data.push_back(std::byte(rawData[i]));
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

bool
Data::operator!=(const Data& other) const noexcept {
  if (size() != other.size()) {
    return true;
  }
  return hashCode() != other.hashCode();
}

bool
Data::operator<(const Data& other) const noexcept
{
  return hashCode() < other.hashCode();
}

bool
Data::operator>(const Data& other) const noexcept
{
  return hashCode() > other.hashCode();
}

std::size_t
Data::hashCode() const noexcept {
  // TODO consider a faster (E.g. SIMD) algorithm or one with less hotspots (see hashdos attacks)
  return std::hash<std::vector<std::byte>>{}(data);
}

std::size_t
Data::size() const noexcept {
  return data.size();
}

void
Data::clear() noexcept
{
  data.clear();
}

} // end namespace
} // end namespace