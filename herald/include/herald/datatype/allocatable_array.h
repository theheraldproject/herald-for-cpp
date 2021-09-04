//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ALLOCATABLE_ARRAY_H
#define HERALD_ALLOCATABLE_ARRAY_H

#include <array>
#include <bitset>

namespace herald {
namespace datatype {

/// FWD DECLARATION
template <typename AllocatableArrayT,
          typename ValT = typename AllocatableArrayT::value_type>
struct AllocatableArrayIterator;

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
  /// \brief The difference type for positions in this container
  using difference_type = std::size_t;
  /// \brief The iterator type for this container
  using iterator = AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>>;
  /// \brief Reference to the size type of this container
  using size_type = std::size_t;
  // using pointer = value_type*;
  // using reference = value_type&;

  /// \brief The maximum size of this container
  static constexpr std::size_t max_size = Size;
  /// \brief Whether this container ensures its contents is unique
  // static constexpr bool EnsureUnique = UniqueMembers;

  /// \brief Default noexcept constructor
  AllocatableArray() noexcept : m_allocated(), m_members() {};
  AllocatableArray(ValT&& first) : m_allocated(), m_members() {
    add(first);
  };
  /// \brief Default noexcept destructor
  ~AllocatableArray() noexcept = default;

  /// \brief Adds an item to this array
  /// If UniqueMembers is true, checks to see if an equivalent item 
  /// already exists, and applies the assignment operator to it.
  /// Returns false if full, rather than throwing an exception.
  bool add(ValT toAdd) noexcept {
    if constexpr (UniqueMembers) {
      const std::size_t equalIdx = findEqual(toAdd);
      if (max_size != equalIdx) {
        m_members[equalIdx] = toAdd; // overwrite existing reference
        return true;
      }
    }
    if (size() == max_size) {
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
    while (max_size != idxToRemove) {
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

  /// \brief Returns the idx'th set value within the AllocatableArray
  value_type& operator[](std::size_t idx) noexcept {
    return m_members[realIndex(idx)];
  }

  /// \brief Support for iterator begin
  AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>> begin() {
    return AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>>(*this);
  }

  // \brief Support for iterator end
  AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>> end() {
    if (size() == 0) return AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>>(*this);
    return AllocatableArrayIterator<AllocatableArray<ValT,Size,UniqueMembers>>(*this, size());
  }

  // TODO consider adding a find method

private:
  std::bitset<max_size> m_allocated;
  std::array<ValT,max_size> m_members;

  /// \brief Returns the next available unassigned index
  /// Returns Size if full
  std::size_t nextAvailable() const noexcept {
    std::size_t idx = 0;
    while (idx < max_size) {
      if (!m_allocated.test(idx)) {
        return idx;
      }
      ++idx;
    }
    return max_size;
  }

  /// \brief Returns the next allocated and equal item's index
  /// Returns Size if none is found
  template <typename EqualT>
  std::size_t findEqual(const EqualT& equiv) const noexcept {
    std::size_t idx = 0;
    while (idx < max_size) {
      if (m_allocated.test(idx)) {
        // EqualT must be either the same as ValT or something ValT has an explicit conversion function for
        if (equiv == (EqualT)m_members[idx]) {
          return idx;
        }
      }
      ++idx;
    }
    return max_size;
  }

  /// \brief Returns the virtualIndex'th set element, or the last set element.
  /// Rather than throwing, if there are no elements in the array, 
  /// it will returns the first instantiated (0'th position) element.
  /// This will likely cause unpredictable behaviour in your app, but not
  /// undefined behaviour in the C++ sense.
  std::size_t realIndex(std::size_t virtualIndex) const noexcept {
    std::size_t idx = 0;
    std::size_t lastMatchedIndex = 0;
    std::size_t count = 0;
    while (idx < max_size) {
      if (m_allocated.test(idx)) {
        lastMatchedIndex = idx;
        if (virtualIndex == count - 1) {
          // return this index
          return idx;
        }
        // Else just increment count
        ++count;
      }
      ++idx;
    }
    return lastMatchedIndex; // Always the last index that is set
  }
};


/// \brief Provides a forward and reverse iterator for an AllocatableArray
/// \brief Implements noexcept guarantees
template <typename AllocatableArrayT,
          typename ValT>
struct AllocatableArrayIterator {
  using difference_type = std::size_t;
  using value_type = ValT;
  using iterator_category = std::forward_iterator_tag;
  using pointer = value_type*;
  using reference = value_type&;

  /// \brief Iterator constructor starting at virtual index 0
  AllocatableArrayIterator(AllocatableArrayT& aa) noexcept : m_aa(aa), pos(0) {}
  /// \brief Iterator constructor starting at virtual index from
  AllocatableArrayIterator(AllocatableArrayT& aa, std::size_t from) noexcept : m_aa(aa), pos(from) {}
  /// \brief Copy constructor
  AllocatableArrayIterator(const AllocatableArrayIterator& other) noexcept : m_aa(other.m_aa), pos(other.pos) {}
  /// \brief Move constructor
  AllocatableArrayIterator(AllocatableArrayIterator&& other) noexcept : m_aa(other.m_aa), pos(other.pos) {}
  /// \brief Default destructor
  ~AllocatableArrayIterator() noexcept = default;

  /// Dereference operator to return the value behind this iterator
  reference operator*() noexcept {
    return m_aa[pos];
  }

  pointer operator->() noexcept {
    return &m_aa[pos];
  }


  /// \brief Allows this iterator to be moved forward
  AllocatableArrayIterator<AllocatableArrayT>& operator+(std::size_t by) noexcept {
    pos += by;
    if (pos > m_aa.size()) {
      pos = m_aa.size();
    }
    return *this;
  }

  /// \brief Allows this iterator to be moved backward
  AllocatableArrayIterator<AllocatableArrayT>& operator-(std::size_t by) noexcept {
    if (by > pos) {
      pos = 0;
    } else {
      pos -= by;
    }
    return *this;
  }

  /// \brief Minus operator to allow std::distance to work
  difference_type operator-(const AllocatableArrayIterator<AllocatableArrayT>& other) noexcept {
    return pos - other.pos;
  }

  /// \brief Prefix increment operator
  AllocatableArrayIterator<AllocatableArrayT>& operator++() noexcept {
    ++pos;
    return *this;
  }

  /// \brief Postfix increment operator
  AllocatableArrayIterator<AllocatableArrayT>& operator++(int) noexcept {
    AllocatableArrayIterator<AllocatableArrayT> cp = *this;
    ++(*this);
    return cp;
  }

  /// \brief Equality operator. Compares position.
  bool operator==(const AllocatableArrayIterator<AllocatableArrayT>& otherIter) const noexcept {
    return pos == otherIter.pos;
  }

  /// \brief Inequality operator. Compares position.
  bool operator!=(const AllocatableArrayIterator<AllocatableArrayT>& otherIter) const noexcept {
    return pos != otherIter.pos;
  }

private:
  AllocatableArrayT& m_aa;
  std::size_t pos;
};

/// \brief Distance operator for std::distance
template <typename T>
typename AllocatableArrayIterator<T>::difference_type distance(AllocatableArrayIterator<T> first, AllocatableArrayIterator<T> last) noexcept {
  return last - first;
}

}
}

#endif