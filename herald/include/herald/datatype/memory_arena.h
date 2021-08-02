//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_MEMORY_ARENA_H
#define HERALD_MEMORY_ARENA_H

#include <cstddef>
#include <bitset>
#include <array>

/// \brief Acts as a non-global memory arena for arbitrary classes
namespace herald {
namespace datatype {

/// \brief Represents an external 'pointer' to an allocated memory area.
///
/// Used by calling classes only.
/// Max memory allocation in pages is 65536
struct MemoryArenaEntry {
  unsigned short startPageIndex;
  unsigned short byteLength;
};

constexpr long pagesRequired(std::size_t size,std::size_t pageSize)
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

  MemoryArena() noexcept
   : arena(), pagesInUse(false)
  {
    ;
  }

  ~MemoryArena() = default;

  MemoryArenaEntry allocate(std::size_t size) {
    // find first page location with enough space
    long pages = pagesRequired(size,PageSize);
    bool inEmpty = false;
    long lastEmptyIndex = 0;
    for (std::size_t i = 0;i < pagesInUse.size();++i) {
      if (!pagesInUse.test(i)) {
        // this one is empty
        if (!inEmpty) {
          inEmpty = true;
          lastEmptyIndex = i;
        } else {
          if (i - lastEmptyIndex + 1 == pages) {
            // flip bits and return
            for (int f = lastEmptyIndex;f <= i;++f) {
              pagesInUse.set(f,true);
            }
            return MemoryArenaEntry{(unsigned short)lastEmptyIndex,(unsigned short)size};
          }
        }
      } else {
        inEmpty = false;
      }
    }
    // ran out of memory! Throw! (Causes catastrophic crash)
    throw std::exception("Unable to allocate memory in arena");
  }

  void deallocate(const MemoryArenaEntry& entry) {
    // set relevant bits to empty
    long pages = pagesRequired(entry.byteLength,PageSize);
    for (int i = 0;i < pages;++i) {
      pagesInUse.set(entry.startPageIndex + i,false);
    }
  }

  void set(const MemoryArenaEntry& entry, unsigned short bytePosition, char value) {
    arena[(entry.startPageIndex * PageSize) + bytePosition] = value;
  }

  char get(const MemoryArenaEntry& entry, unsigned short bytePosition) {
    return arena[(entry.startPageIndex * PageSize) + bytePosition];
  }

  std::size_t pagesFree() const {
    return pagesRequired(Size,PageSize) - pagesInUse.count();
  }

private:
  std::array<char,Size> arena;
  std::bitset<pagesRequired(Size,PageSize)> pagesInUse;
};

}
}

#endif