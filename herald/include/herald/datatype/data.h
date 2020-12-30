//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>
#include <memory>

namespace herald {
namespace datatype {

class Data {
public:
  Data();
  Data(Data&& other); // move ctor
  Data(const std::byte* value, std::size_t length);
  Data(std::vector<std::byte> value);
  Data(const Data& from); // copy ctor
  Data(std::byte repeating, std::size_t count);
  // Data(Base64String from); // use Base64String.from(std::string).decode() instead
  Data& operator=(const Data& other);
  ~Data();

  // std::string base64EncodedString(); // use Base64String.encode(Data) instead

  std::string description() const;

  Data subdata(std::size_t offset) const;
  Data subdata(std::size_t offset, std::size_t length) const;
  std::byte at(std::size_t index) const;
  void append(const Data& data);
  void append(uint8_t data);
  void append(uint16_t data);
  void append(uint32_t data);
  void append(uint64_t data);
  void append(const std::string& data);
  bool uint8(std::size_t fromIndex, uint8_t& into) const noexcept;
  bool uint16(std::size_t fromIndex, uint16_t& into) const noexcept;
  bool uint32(std::size_t fromIndex, uint32_t& into) const noexcept;
  bool uint64(std::size_t fromIndex, uint64_t& into) const noexcept;
  bool operator==(const Data& other) const noexcept;

  std::size_t hashCode() const;

  std::size_t size() const;

protected:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};

} // end namespace
} // end namespace

namespace std {
  // std::hash function for std::vector<T>
  template<typename T>
  struct hash<std::vector<T>>
  {
    size_t operator()(const std::vector<T>& v) const
    {
      std::size_t hv = 0;
      for (auto& vpart : v) {
        hv = hv ^ (((char)vpart) << 1);
      }
      return hv;
    }
  };
} // end namespace

#endif