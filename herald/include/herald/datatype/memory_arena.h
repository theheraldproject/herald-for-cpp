//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_MEMORY_ARENA_H
#define HERALD_MEMORY_ARENA_H

#include <cstddef>
#include <bitset>
#include <array>
#include <limits>

/// \brief Acts as a non-global memory arena for arbitrary classes
namespace herald {
namespace datatype {

/// \brief Represents an external 'pointer' to an allocated memory area.
///
/// Used by calling classes only.
/// Max memory allocation in bytes is 65536
struct MemoryArenaEntry {
  unsigned short startPageIndex = 0;
  unsigned short byteLength = 0;

  bool isInitialised() const {
    return 0 != byteLength;
  }
};

constexpr unsigned long pagesRequired(std::size_t size,std::size_t pageSize)
{
  return (size + pageSize - 1) / pageSize;
}

/// \brief Very basic paged memory arena class
///
/// Can be used one arena per dynamic allocation class, or used by multiple classes.
/// In this non-global implementation, pass it as a static reference variable to the class
/// once during application startup after allocation in a main class or similar.
template <std::size_t MaxSize, std::size_t AllocationSize>
class MemoryArena {
public:
  /// \brief The Maximum size to use for data (doesn't include page table)
  static constexpr std::size_t Size = MaxSize;
  /// \brief The allocation size for each page. Note total mem usage is Size + (Size / PageSize)
  /// Thus for MemoryArena<2048,10>() you use 2048 + (2048 / 10) = 2253 bytes
  static constexpr std::size_t PageSize = AllocationSize;

  constexpr MemoryArena() noexcept
   : arena(), pagesInUse(false)
  {
    ;
  }

  ~MemoryArena() noexcept = default;

  /// \brief Forces all pages to be unset. Effectively clears memory in use.
  void reset() noexcept {
    pagesInUse.reset();
  }

  void reserve(MemoryArenaEntry& entry,std::size_t newSize) noexcept {
    if (newSize <= entry.byteLength) {
      return;
    }
    auto newEntry = allocate(newSize);
    for (std::size_t i = 0;i < entry.byteLength;++i) {
      set(newEntry,i,get(entry,i));
    }
    deallocate(entry);
    entry = newEntry;
  }

  MemoryArenaEntry allocate(std::size_t size) {
    if (0 == size) {
      return MemoryArenaEntry{0,0};
    }
    // find first page location with enough space
    unsigned long pages = pagesRequired(size,PageSize);
    bool inEmpty = false;
    unsigned long lastEmptyIndex = 0;
    for (std::size_t i = 0;i < pagesInUse.size();++i) {
      if (!pagesInUse.test(i)) {
        // this one is empty
        if (!inEmpty) {
          inEmpty = true;
          lastEmptyIndex = i;
        }
        if (i - lastEmptyIndex + 1 == pages) {
          // flip bits and return
          for (unsigned long f = lastEmptyIndex;f <= i;++f) {
            pagesInUse.set(f,true);
          }
          return MemoryArenaEntry{(unsigned short)lastEmptyIndex,(unsigned short)size};
        }
      } else {
        inEmpty = false;
      }
    }
    // ran out of memory! Throw! (Causes catastrophic crash)
#ifdef __ZEPHYR__
    std::terminate();
#else
    throw std::runtime_error("Unable to allocate memory in arena");
#endif
  }

  void deallocate(MemoryArenaEntry& entry) noexcept {
    if (!entry.isInitialised()) {
      return; // guard
    }
    // set relevant bits to empty
    long pages = pagesRequired(entry.byteLength,PageSize);
    for (int i = 0;i < pages;++i) {
      pagesInUse.set(entry.startPageIndex + i,false);
    }
    entry.byteLength = 0;
    entry.startPageIndex = 0;
  }

  void set(const MemoryArenaEntry& entry, unsigned short bytePosition, unsigned char value) noexcept {
    if (!entry.isInitialised()) {
      return;
    }
    arena[(entry.startPageIndex * PageSize) + bytePosition] = value;
  }

  char get(const MemoryArenaEntry& entry, unsigned short bytePosition) const noexcept {
    if (!entry.isInitialised()) {
      return '\0';
    }
    return arena[(entry.startPageIndex * PageSize) + bytePosition];
  }

  const unsigned char* rawStartAddress(const MemoryArenaEntry& entry) const noexcept {
    if (!entry.isInitialised()) {
      return 0;
    }
    // The following is guaranteed by the STL 23.3.2.1
    return &arena[(entry.startPageIndex * PageSize)];
  }

  std::size_t pagesFree() const noexcept {
    return pagesRequired(Size,PageSize) - pagesInUse.count();
  }

  /// \brief Copies the segment of raw data from this arena
  /// \note A template, so if unused in production it compiles out to nothing (zero overhead)
  template <typename ByteArrayT>
  void rawCopy(ByteArrayT& toPopulate, const std::size_t offset) noexcept {
    const std::size_t len = ((Size - offset) < toPopulate.size()) ? (Size - offset) : toPopulate.size();
    std::size_t pos = 0;
    std::size_t page = 0;
    for (;(pos < len) && ((offset + pos) < Size);++pos) {
      // Note: If deallocated, return 00 always (so as not to confuse the reader) - we don't explicitly zero out memory after use!
      page = (offset + pos) / PageSize;
      if (!pagesInUse.test(page)) {
        // Not in use, return a 0
        toPopulate[pos] = std::numeric_limits<unsigned char>::min();
      } else {
        toPopulate[pos] = arena[offset + pos];
      }
    }
    // we may have ran out of bytes in the arena, so just return empty bytes in the destination array
    for (;pos < len;++pos) {
      toPopulate[pos] = std::numeric_limits<unsigned char>::min();
    }
  }

  std::size_t size() const noexcept {
    return Size;
  }

private:
  std::array<unsigned char,Size> arena;
  std::bitset<pagesRequired(Size,PageSize)> pagesInUse;
};

}
}

#endif