//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DATA_H
#define HERALD_DATA_H

#include <vector>
#include <string>
#include <memory>
#include <iostream>

namespace herald {
namespace datatype {

/// \brief The main data workhorse class of the Herald API
///
/// A vitally important part of Herald's datatypes. Many other types
/// are actually just aliases or thin wrappers to the Data class.
/// 
/// This class represents an arbitrarily long Big Endian list of std::byte.
/// Data instances are used to encode Bluetooth advert data, to pass payloads
/// between herald enabled devices, or to share data to and from backend systems.
class Data {
public:
  Data();
  Data(Data&& other); // move ctor
  Data(const std::uint8_t* value, std::size_t length);
  Data(const std::byte* value, std::size_t length);
  Data(std::vector<std::byte> value);
  Data(const Data& from); // copy ctor
  Data(std::byte repeating, std::size_t count);
  Data(std::size_t reserveLength);
  // Data(Base64String from); // use Base64String.from(std::string).decode() instead
  Data& operator=(const Data& other);
  ~Data();

  // std::string base64EncodedString(); // use Base64String.encode(Data) instead
  static Data fromHexEncodedString(const std::string& hex);

  std::string description() const;

  Data subdata(std::size_t offset) const;
  Data subdata(std::size_t offset, std::size_t length) const;
  std::byte at(std::size_t index) const;
  /// Assigns the data in-place, reserving the size required if the current size is too small.
  /// Avoids reallocation of memory on a copy.
  void assign(const Data& other);
  void append(const Data& data, std::size_t offset, std::size_t length);
  void append(const std::uint8_t* data, std::size_t offset, std::size_t length);
  void appendReversed(const Data& data, std::size_t offset, std::size_t length);
  void append(const Data& data);
  void append(std::byte data);
  void append(uint8_t data);
  void append(uint16_t data);
  void append(uint32_t data);
  void append(uint64_t data);
  void append(const std::string& data);
  bool uint8(std::size_t fromIndex, uint8_t& into) const noexcept;
  bool uint16(std::size_t fromIndex, uint16_t& into) const noexcept;
  bool uint32(std::size_t fromIndex, uint32_t& into) const noexcept;
  bool uint64(std::size_t fromIndex, uint64_t& into) const noexcept;
  // TODO signed versions of the above functions too
  bool operator==(const Data& other) const noexcept;
  bool operator!=(const Data& other) const noexcept;
  bool operator<(const Data& other) const noexcept; // required for std::less
  bool operator>(const Data& other) const noexcept; // required for std::less

  Data reversed() const;

  std::string hexEncodedString() const noexcept;

  std::size_t hashCode() const noexcept;

  std::size_t size() const noexcept;
  // TODO support other C++ STD container type functions to allow iteration over data elements (uint8)

  void clear() noexcept;
  
protected:
  static const char hexChars[];
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};

} // end namespace
} // end namespace

namespace std {
  inline std::ostream& operator<<(std::ostream &os, const herald::datatype::Data& d)
  {
    return os << d.hexEncodedString();
  }

  inline void hash_combine_impl(std::size_t& seed, std::size_t value)
  {
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }
  
  // std::hash function for std::vector<T>
  template<typename T>
  struct hash<std::vector<T>>
  {
    size_t operator()(const std::vector<T>& v) const
    {
      std::size_t hv = 0;
      for (auto& vpart : v) {
        hash_combine_impl(hv, std::hash<T>()(vpart));
      }
      return hv;
    }
  };

  template<>
  struct hash<herald::datatype::Data>
  {
    size_t operator()(const herald::datatype::Data& v) const
    {
      return v.hashCode();
    }
  };
} // end namespace

#endif