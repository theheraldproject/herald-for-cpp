//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DATA_H
#define HERALD_DATA_H

#include <string>
#include <iostream>

#include "memory_arena.h"

namespace herald {
namespace datatype {

/// \brief The main data workhorse class of the Herald API
///
/// A vitally important part of Herald's datatypes. Many other types
/// are actually just aliases or thin wrappers to the DataRef class.
/// 
/// This class represents an arbitrarily long Big Endian list of std::byte.
/// DataRef instances are used to encode Bluetooth advert data, to pass payloads
/// between herald enabled devices, or to share data to and from backend systems.
#ifndef HERALD_MEMORYARENA_MAX
template <typename MemoryArenaT = MemoryArena<8192, 8>>
#else
#ifndef HERALD_MEMORYARENA_PAGE
template <typename MemoryArenaT = MemoryArena<HERALD_MEMORYARENA_MAX, 8>>
#else
template <typename MemoryArenaT = MemoryArena<HERALD_MEMORYARENA_MAX, HERALD_MEMORYARENA_PAGE>>
#endif
#endif
class DataRef {
public:
  /// \brief Creates a DataRef with no memory used
  DataRef() : entry()
  {
    ;
  }
  /// \brief Takes control of another DataRef's memory allocation
  DataRef(DataRef&& other)
  : entry()
  {
    std::swap(entry,other.entry);
  }
  /// \brief Initialises a DataRef from a std::uint8_t array of length `length`
  DataRef(const std::uint8_t* value, std::size_t length) : 
  entry(arena.allocate(length)) {
    for (std::size_t i = 0;i < length; ++i) {
      // data[i] = std::byte(value[i]);
      arena.set(entry, i, (unsigned char)value[i]);
    }
  }
  /// \brief Initialises a DataRef from a std::byte array of length `length`
  DataRef(const std::byte* value, std::size_t length) : entry(arena.allocate(length)) {
    for (std::size_t i = 0;i < length; ++i) {
      // data[i] = value[i];
      arena.set(entry, i, (unsigned char)value[i]);
    }
  }
  /// \brief Initialises a DataRef from a string of chars
  DataRef(const std::string& from) : entry(arena.allocate(from.size())) {
    for (std::size_t i = 0;i < from.size(); ++i) {
      // data[i] = value[i];
      arena.set(entry, i, (unsigned char)from[i]);
    }
  }
  /// \brief Initialises a DataRef copying another data object (uses more data, to ensure only one object owns the entry)
  DataRef(const DataRef& from) : entry(arena.allocate(from.entry.byteLength)) {
    for (std::size_t i = 0;i < from.size(); ++i) {
      arena.set(entry, i, from.arena.get(from.entry,i));
    }
  }

  /// \brief Initialises a DataRef with count number of repeating bytes
  DataRef(std::byte repeating, std::size_t count) : entry(arena.allocate(count)) {
    for (std::size_t i = 0;i < count; ++i) {
      arena.set(entry,i,(unsigned char)repeating);
    }
  }
  /// \brief Initialises a DataRef with reserveLength bytes of undefined data
  DataRef(std::size_t reserveLength) : entry(arena.allocate(reserveLength)) {
    ;
  }


  // Data(Base64String from); // use Base64String.from(std::string).decode() instea
  /// \brief Copy assign operator. Copies the data to be sure only one object owns the entry
  DataRef& operator=(const DataRef& other)
  {
    entry = arena.allocate(other.entry.byteLength);
    for (std::size_t i = 0;i < other.size(); ++i) {
      arena.set(entry, i, other.arena.get(other.entry,i));
    }
    return *this;
  }

  /// \brief Default destructor
  ~DataRef() {
    clear();
  }

  // std::string base64EncodedString(); // use Base64String.encode(Data) instead
  /// \brief Creates a new DataRef object from a hexadecimal encoded string
  static DataRef fromHexEncodedString(const std::string& hex)
  {
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

    DataRef d(hexInput.size() / 2);

    for (std::size_t i = 0; i < hexInput.size(); i += 2) {
      std::string byteString = hexInput.substr(i, 2);
      std::byte byte = std::byte(strtol(byteString.c_str(), NULL, 16));
      // d.data.push_back(byte);
      arena.set(d.entry,i / 2, (unsigned char)byte);
    }

    return d;
  }

  /// \brief Returns the hex encoded string represetation as the description
  std::string description() const
  {
    return hexEncodedString();
  }

  /// \brief Returns a NEWLY allocated DataRef instance returning a subset of this instance
  DataRef subdata(std::size_t offset) const
  {
    if (offset >= entry.byteLength) {
      return DataRef(0);
    }
    DataRef copy(entry.byteLength - offset);
    for (std::size_t i = 0;i < entry.byteLength - offset;++i) {
      copy.arena.set(copy.entry,i,arena.get(entry,i + offset));
    }
    // std::copy(data.begin() + offset, data.end(), std::back_inserter(copy.data));
    return copy;
  }

  /// \brief Returns a NEWLY allocated DataRef instance returning a subset of this instance
  DataRef subdata(std::size_t offset, std::size_t length) const
  {
    // Note: offset if passed as -1 could be MAX_LONG_LONG, so check it on its own too
    if (offset >= entry.byteLength) {
      return DataRef(0);
    }
    std::size_t correctedLength = length;
    if (length > entry.byteLength || length + offset > entry.byteLength) {
      correctedLength = entry.byteLength - offset;
    }
    DataRef copy(correctedLength);
    // Note the below is necessary as calling with (4,-1), the second condition IS valid!
    // if (length > entry.byteLength || offset + length > entry.byteLength) {
    //   for (std::size_t i = 0;i < entry.byteLength - offset;++i) {
    //     copy.arena.set(copy.entry,i,arena.get(entry,offset + i));
    //   }
    //   // std::copy(data.begin() + offset, data.end(), std::back_inserter(copy.data));
    // } else {
      for (std::size_t i = 0;i < correctedLength;++i) {
        copy.arena.set(copy.entry,i,arena.get(entry,offset + i));
      }
      // std::copy(data.begin() + offset, data.begin() + offset + length, std::back_inserter(copy.data));
    // }
    return copy;
  }

  /// \brief Returns the individual byte at index position, or a byte value of zero if index is out of bounds.
  std::byte at(std::size_t index) const {
    if (index > (unsigned short)(entry.byteLength - 1)) {
      return std::byte(0);
    }
    return std::byte(arena.get(entry,index));
  }

  /// \brief 
  /// Assigns the data in-place, reserving the size required if 
  /// the current size is too small.
  ///
  /// Avoids repeated reallocation of memory on a copy.
  void assign(const DataRef& other)
  {
    if (other.size() > entry.byteLength) {
      arena.reserve(entry,other.size());
    }
    for (std::size_t pos = 0; pos < other.size();++pos) {
      arena.set(entry,pos,other.arena.get(other.entry,pos));
    }
  }

  /// \brief Copies another DataRef into this instance, expanding if required
  void append(const DataRef& rawData, std::size_t offset, std::size_t length)
  {
    auto curSize = entry.byteLength;
    arena.reserve(entry,curSize + length);
    for (std::size_t pos = 0; pos < length;++pos) {
      arena.set(entry,curSize + pos,rawData.arena.get(rawData.entry,pos + offset));
    }
    // std::copy(rawData.data.begin() + offset, 
    //           rawData.data.begin() + offset + length, 
    //           std::back_inserter(data)
    // );
  }

  /// \brief Appends a set of characters to the end of this DataRef
  void append(const std::string& rawData)
  {
    auto curSize = entry.byteLength;
    arena.reserve(entry,curSize + rawData.size());
    for (std::size_t pos = 0; pos < rawData.size();++pos) {
      arena.set(entry,curSize + pos,rawData[pos]);
    }
  }

  /// \brief Copies a uint8_t array onto the end of this instance, expanding if necessary
  void append(const std::uint8_t* rawData, std::size_t offset, std::size_t length)
  {
    auto curSize = entry.byteLength;
    arena.reserve(entry,curSize + length);
    for (std::size_t i = 0;i < length;++i) {
      arena.set(entry,curSize + i,(unsigned char)(rawData[offset + i]));
      // arena.set(entry,curSize,std::byte(rawData[offset + i]));
    }
  }

  /// \brief Appends the specified DataRef to this one, but in its reverse order
  void appendReversed(const DataRef& rawData, std::size_t offset, std::size_t length)
  {
    if (offset > rawData.size()) {
      return; // append nothing - out of range
    }
    std::size_t checkedLength = length;
    if (length > (rawData.size() - offset)) {
      checkedLength = rawData.size() - offset;
    }
    auto curSize = entry.byteLength;
    arena.reserve(entry,curSize + checkedLength);
    for (std::size_t i = 0;i < checkedLength;++i) {
      arena.set(entry,curSize + i,
      rawData.arena.get(rawData.entry,offset + (checkedLength - i - 1)));
    // std::reverse_copy(rawData.data.begin() + offset, 
    //                   rawData.data.begin() + offset + checkedLength, 
    //                   std::back_inserter(data)
    // );
    }
  }

  /// \brief Appends the specified DataRef to this one
  void append(const DataRef& rawData)
  {
    auto orig = entry.byteLength;
    arena.reserve(entry,rawData.size() + orig);
    for (std::size_t pos = 0; pos < rawData.size();++pos) {
      arena.set(entry,orig + pos,rawData.arena.get(rawData.entry,pos));
    }
    // std::copy(rawData.data.begin(), rawData.data.end(), std::back_inserter(data));
  }

  /// \brief Appends a single byte
  void append(std::byte rawData)
  {
    std::size_t curSize = entry.byteLength;
    arena.reserve(entry,curSize + 1);
    // data.push_back(rawData);
    arena.set(entry,curSize,(unsigned char)rawData);
    // curSize++;
  }

  /// \brief appends a single uint8_t
  void append(uint8_t rawData)
  {
    std::size_t curSize = entry.byteLength;
    arena.reserve(entry,curSize + 1); // C++ ensures types are AT LEAST x bits
    // arena.set(entry,curSize,std::byte(rawData));
    arena.set(entry,curSize,(unsigned char)(rawData));
    // curSize++;
  }

  /// \brief Appends a single uint16_t
  void append(uint16_t rawData)
  {
    std::size_t curSize = entry.byteLength;
    arena.reserve(entry,curSize + 2); // C++ ensures types are AT LEAST x bits
    arena.set(entry,curSize,(unsigned char)(rawData & 0xff));
    arena.set(entry,curSize + 1,(unsigned char)(rawData >> 8));
  }

  /// \brief Appends a single uint32_t
  void append(uint32_t rawData)
  {
    std::size_t curSize = entry.byteLength;
    arena.reserve(entry,curSize + 4); // C++ ensures types are AT LEAST x bits
    arena.set(entry,curSize,(unsigned char)(rawData & 0xff));
    arena.set(entry,curSize + 1,(unsigned char)(rawData >> 8));
    arena.set(entry,curSize + 2,(unsigned char)(rawData >> 16));
    arena.set(entry,curSize + 3,(unsigned char)(rawData >> 24));
  }

  /// \brief Appends a single uint64_t
  void append(uint64_t rawData)
  {
    std::size_t curSize = entry.byteLength;
    arena.reserve(entry,curSize + 8); // C++ ensures types are AT LEAST x bits
    arena.set(entry,curSize,(unsigned char)(rawData & 0xff));
    arena.set(entry,curSize + 1,(unsigned char)(rawData >> 8));
    arena.set(entry,curSize + 2,(unsigned char)(rawData >> 16));
    arena.set(entry,curSize + 3,(unsigned char)(rawData >> 24));
    arena.set(entry,curSize + 4,(unsigned char)(rawData >> 32));
    arena.set(entry,curSize + 5,(unsigned char)(rawData >> 40));
    arena.set(entry,curSize + 6,(unsigned char)(rawData >> 48));
    arena.set(entry,curSize + 7,(unsigned char)(rawData >> 56));
  }

  /// \brief Returns whether reading a single uint8_t to `into` at `fromIndex` was successful
  bool uint8(std::size_t fromIndex, uint8_t& into) const noexcept
  {
    if (fromIndex > (unsigned short)(entry.byteLength - 1)) {
      return false;
    }
    into = std::uint8_t(arena.get(entry,fromIndex));
    return true;
  }

  /// \brief Returns whether reading a single uint16_t to `into` at `fromIndex` was successful
  bool uint16(std::size_t fromIndex, uint16_t& into) const noexcept
  {
    if (fromIndex > (unsigned short)(entry.byteLength - 2)) {
      return false;
    }
    into = (std::uint16_t(std::uint8_t(arena.get(entry,fromIndex + 1))) << 8) | std::uint16_t(std::uint8_t(arena.get(entry,fromIndex)));
    return true;
  }

  /// \brief Returns whether reading a single uint32_t to `into` at `fromIndex` was successful
  bool uint32(std::size_t fromIndex, uint32_t& into) const noexcept
  {
    if (fromIndex > entry.byteLength - 4) {
      return false;
    }
    into =  std::uint32_t(std::uint8_t(arena.get(entry,fromIndex)))            | (std::uint32_t(std::uint8_t(arena.get(entry,fromIndex + 1))) << 8) |
          (std::uint32_t(std::uint8_t(arena.get(entry,fromIndex + 2))) << 16) | (std::uint32_t(std::uint8_t(arena.get(entry,fromIndex + 3))) << 24);
    return true;
  }

  /// \brief Returns whether reading a single uint64_t to `into` at `fromIndex` was successful
  bool uint64(std::size_t fromIndex, uint64_t& into) const noexcept
  {
    if (entry.byteLength < 8 || fromIndex > entry.byteLength - 8) {
      return false;
    }
    into = (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 7))) << 56) | (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 6))) << 48) |
          (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 5))) << 40) | (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 4))) << 32) |
          (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 3))) << 24) | (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 2))) << 16) |
          (std::uint64_t(std::uint8_t(arena.get(entry,fromIndex + 1))) << 8)  |  std::uint64_t(std::uint8_t(arena.get(entry,fromIndex)));
    return true;
  }

  // TODO signed versions of the above functions too
  /// \brief Equality operator for another DataRef instance (same memory arena)
  bool operator==(const DataRef& other) const noexcept
  {
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

  /// \brief Inequality operator for another DataRef instance (same memory arena)
  bool operator!=(const DataRef& other) const noexcept
  {
    if (size() != other.size()) {
      return true;
    }
    return hashCode() != other.hashCode();
  }

  /// \brief Less than operator for another DataRef instance (same memory arena)
  bool operator<(const DataRef& other) const noexcept
  {
    return hashCode() < other.hashCode();
  }

  /// \brief Greater than operator for another DataRef instance (same memory arena)
  bool operator>(const DataRef& other) const noexcept
  {
    return hashCode() > other.hashCode();
  }

  /// \brief Returns a new DataRef instance with the same data as this one, but in the reverse order
  DataRef reversed() const
  {
    DataRef result(entry.byteLength);
    // result.reserve(entry.byteLength);
    for (std::size_t pos = 0;pos < entry.byteLength;++pos) {
      result.arena.set(result.entry,pos,arena.get(entry,entry.byteLength - pos - 1));
    }
    // std::reverse_copy(data.begin(),data.end(),
    //   std::back_inserter(result.data)
    // );
    return result;
  }

  /// \brief Returns the same order of bytes, but with the bits in each byte reversed
  DataRef reverseEndianness() const
  {
    DataRef result(entry.byteLength);
    // result.data.reserve(entry.byteLength);

    // Keep byte order intact (caller could use reversed() to change that)
    // but reverse the order of the individual bits by each byte
    std::uint8_t value, original;
    for (std::size_t i = 0;i < entry.byteLength;++i) {
      original = std::uint8_t(arena.get(entry,i));
      value = 0;
      for (int b = 0;b < 8;++b) {
        if ((original & (1 << b)) > 0) {
          value |= 1 << (7 - b);
        }
      }
      // result.data[i] = std::byte(value);
      result.arena.set(result.entry,entry.byteLength - i - 1,value);
    }

    return result;
  }

  /// \brief Returns a hex encoded string of this binary data
  std::string hexEncodedString() const noexcept
  {
    if (0 == entry.byteLength) {
      return "";
    }
    std::string result;
    std::size_t size = entry.byteLength;
    result.reserve(size * 2);
    std::size_t v;
    for (std::size_t i = 0; i < size; ++i) {
      // v = std::size_t(data.at(i));
      v = std::size_t(arena.get(entry,i));
      result += hexChars[0x0F & (v >> 4)]; // MSB
      result += hexChars[0x0F &  v      ]; // LSB
    }
    return result;
  }

  /// \brief Returns the hash code of this instance
  std::size_t hashCode() const noexcept
  {
    // TODO consider a faster (E.g. SIMD) algorithm or one with less hotspots (see hashdos attacks)
    return std::hash<DataRef<MemoryArenaT>>{}(*this);
  }

  /// \brief Returns the size in allocated bytes of this instance
  std::size_t size() const noexcept{
    return entry.byteLength;
  }
  // TODO support other C++ STD container type functions to allow iteration over data elements (uint8)

  /// \brief Clears (deallocates) the bytes referred to by this instance
  void clear() noexcept
  {
    arena.deallocate(entry);
  }

  unsigned char* rawMemoryStartAddress() const {
    return arena.rawStartAddress(entry);
  }

  static MemoryArenaT& getArena() {
    return arena;
  }
  
protected:
  static const char hexChars[];
  static MemoryArenaT arena;
  MemoryArenaEntry entry;
};




/// \brief Instantiates the MemoryArena instance used by all DataRefs that share it
template <typename MemoryArenaT>
MemoryArenaT DataRef<MemoryArenaT>::arena = MemoryArenaT();

template <typename MemoryArenaT>
const char DataRef<MemoryArenaT>::hexChars[] = {
  '0','1','2','3','4','5','6','7',
  '8','9','a','b','c','d','e','f'
};

/// \brief Defaults references to Data to equal the DataRef with the default Memory Arena dimensions, unless HERALD_MEMORYARENA_MAX is specified
/// May also have its allocation size set by HERALD_MEMORYARENA_PAGE. Note this only takes
/// affect if HERALD_MEMORYARENA_MAX is also specified.
using Data = DataRef<>; // uses MemoryArenaT<4096,8>




/// \brief Represents a fixed array of Data references using the default memory arena that tracks its own in-use size
template <std::size_t maxSize = 8>
class DataSections {
public:
  static constexpr std::size_t MaxSize = maxSize;

  /// \brief Default constructors. No memory allocation other than size (8 bytes)
  DataSections() noexcept : sections(), sz(0) {}
  /// \brief Default destructor
  ~DataSections() noexcept = default;

  /// \brief Adds a new section, if room exists. Otherwise quietly performs a NO OP.
  void append(const Data& toCopy) noexcept {
    if (sz >= MaxSize) {
      return;
    }
    sections[sz] = toCopy; // copy assign operator
    ++sz;
  }

  /// \brief Returns the number of dataref elements in use
  std::size_t size() noexcept {
    return sz;
  }

  /// \brief Returns the DataRef at the relevant index, or an empty DataRef
  const Data& get(std::size_t index) noexcept {
    if (index >= MaxSize) {
      return emptyRef;
    }
    return sections[index];
  }

  /// \brief Returns the DataRef at the relevant index, or an empty DataRef
  const Data& operator[](std::size_t index) noexcept {
    return get(index);
  }

private:
  static Data emptyRef;
  std::array<Data,MaxSize> sections;
  std::size_t sz;
};

/// \brief Reference to empty data item for optional/empty return by ref
template <std::size_t maxSize>
Data DataSections<maxSize>::emptyRef = Data();

} // end namespace
} // end namespace

namespace std {
  template <typename MemoryArenaT>
  inline std::ostream& operator<<(std::ostream &os, const herald::datatype::DataRef<MemoryArenaT>& d)
  {
    return os << d.hexEncodedString();
  }

  inline void hash_combine_impl(std::size_t& seed, std::size_t value)
  {
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  template<typename MemoryArenaT>
  struct hash<herald::datatype::DataRef<MemoryArenaT>>
  {
    size_t operator()(const herald::datatype::DataRef<MemoryArenaT>& v) const
    {
      std::size_t hv = 0;
      std::uint8_t ui = 0;
      bool ok;
      for (std::size_t pos = 0;pos < v.size();++pos) {
        ok = v.uint8(pos,ui);
        hash_combine_impl(hv, std::hash<std::uint8_t>()(ui));
      }
      return hv;
    }
  };
} // end namespace

#endif