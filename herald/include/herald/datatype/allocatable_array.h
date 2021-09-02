//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ALLOCATABLE_ARRAY_H
#define HERALD_ALLOCATABLE_ARRAY_H

#include <array>
#include <bitset>

namespace herald {
namespace datatype {

/// \brief An array with a max size at compile time, but a tracked max size at runtime.
/// \since v2.1.0
/// This class is noexcept compliant.
/// This class defaults to a container size of 8, but this can be changed.
/// This class also defaults to ensuring UniqueMembers, but this too can be changed.
template <typename ValT, std::size_t Size = 8, bool UniqueMembers = true>
class AllocatableArray {
public:
  /// \brief Reference to the value type within this container
  using value_type = ValT;
  /// \brief Reference to the size type of this container
  using size_type = std::size_t;

  /// \brief The maximum size of this container
  static constexpr std::size_t MaxSize = Size;
  /// \brief Whether this container ensures its contents is unique
  // static constexpr bool EnsureUnique = UniqueMembers;

  /// \brief Default noexcept constructor
  AllocatableArray() noexcept : m_allocated(), m_members() {};
  /// \brief Default noexcept destructor
  ~AllocatableArray() noexcept = default;

  /// \brief Adds an item to this array
  /// If UniqueMembers is true, checks to see if an equivalent item 
  /// already exists, and applies the assignment operator to it.
  /// Returns false if full, rather than throwing an exception.
  bool add(ValT&& toAdd) noexcept {
    if constexpr (UniqueMembers) {
      const std::size_t equalIdx = findEqual(toAdd);
      if (MaxSize != equalIdx) {
        m_members[equalIdx] = toAdd; // overwrite existing reference
        return true;
      }
    }
    if (size() == MaxSize) {
      return false;
    }
    const std::size_t nextIdx = nextAvailable();
    m_members[nextIdx] = toAdd;
    m_allocated.set(nextIdx);
    return true;
  }

  /// \brief Removes (all instances of, if UniqueMembers is false) an item from this array
  /// \note This applies the equality operator of the passed in 
  ///       value to each allocated object.
  template <typename EqualT>
  void remove(const EqualT& removeIfEqual) noexcept {
    std::size_t idxToRemove = findEqual(removeIfEqual);
    while (MaxSize != idxToRemove) {
      m_allocated.set(idxToRemove,false);
      if constexpr (UniqueMembers) {
        return; // small performance enhancement
      }
      idxToRemove = findEqual(removeIfEqual);
    }
  }

  /// \brief Returns the current assigned size of this array
  std::size_t size() const noexcept {
    return m_allocated.count();
  }

  // TODO support noexcept iterators and iterator type constexpr/using

  // TODO consider adding a find method

private:
  std::bitset<MaxSize> m_allocated;
  std::array<ValT,MaxSize> m_members;

  /// \brief Returns the next available unassigned index
  /// Returns MaxSize if full
  std::size_t nextAvailable() const noexcept {
    std::size_t idx = 0;
    while (idx < MaxSize) {
      if (!m_allocated.test[idx]) {
        return idx;
      }
      ++idx;
    }
    return MaxSize;
  }

  /// \brief Returns the next allocated and equal item's index
  /// Returns MaxSize if none is found
  template <typename EqualT>
  std::size_t findEqual(const EqualT& equiv) noexcept {
    std::size_t idx = 0;
    while (idx < MaxSize) {
      if (m_allocated.test[idx]) {
        if (equiv == m_allocated[idx]) {
          return idx;
        }
      }
      ++idx;
    }
    return MaxSize;
  }
};

}
}

#endif