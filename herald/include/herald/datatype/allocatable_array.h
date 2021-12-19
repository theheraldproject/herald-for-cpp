//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ALLOCATABLE_ARRAY_H
#define HERALD_ALLOCATABLE_ARRAY_H

#include "../util/is_valid.h"

#include <array>
#include <bitset>
#include <type_traits>
#include <optional>
#include <functional>

namespace herald {
namespace datatype {

/// FWD DECLARATION
template <typename AllocatableArrayT,
          typename ValT = typename AllocatableArrayT::value_type, 
          typename ValBaseT = typename std::remove_cv<ValT>::type
         >
struct AllocatableArrayIterator;
template <typename AllocatableArrayT,
          typename ValT = typename AllocatableArrayT::value_type, 
          typename ValBaseT = typename std::remove_cv<ValT>::type
         >
struct ConstAllocatableArrayIterator;

/// \brief An array with a max size at compile time, but a tracked max size at runtime.
/// \since v2.1.0
///
/// This container provides a fixed max-size container whilst managing the assigned/unassigned
/// nature internally to the container, and not exposing unassigned elements via its
/// iterators.
///
/// A specialisation is provided for arrays of references whose types cannot be default constructed
/// and thus is an array of std::optional of std::reference_wrapper of ValT. This is called
/// ReferenceArray.
///
/// This class is noexcept compliant.
/// This class defaults to a container size of 8, but this can be changed.
/// This class also defaults to ensuring UniqueMembers, but this too can be changed.
template <typename ValT, std::size_t Size = 8, bool UniqueMembers = true, typename ValBaseT = typename std::remove_cv<ValT>::type>
class AllocatableArray {
public:
  /// \brief Reference to the value type within this container
  using value_type = ValBaseT; // must be non-const
  /// \brief The difference type for positions in this container
  using difference_type = std::size_t;
  /// \brief The iterator type for this container
  using iterator = AllocatableArrayIterator<AllocatableArray<ValBaseT,Size,UniqueMembers>>;
  /// \brief Constant iterator type
  using const_iterator = ConstAllocatableArrayIterator<AllocatableArray<ValBaseT,Size,UniqueMembers>>;;
  /// \brief Reference to the size type of this container
  using size_type = std::size_t;
  // using pointer = value_type*;
  // using reference = value_type&;

  /// \brief The maximum size of this container
  static constexpr std::size_t max_size = Size;
  /// \brief Whether this container ensures its contents is unique
  static constexpr bool unique_members = UniqueMembers;

  /// \brief Default noexcept constructor
  AllocatableArray() noexcept : m_allocated(), m_members() {};
  /// \brief Copy constructor (relies on std::array copy constructor)
  AllocatableArray(const AllocatableArray& from) noexcept : m_allocated(from.m_allocated), m_members(from.m_members) {};
  /// \brief Single value constructor for convenience
  AllocatableArray(ValBaseT&& first) noexcept : m_allocated(), m_members() {
    add(first);
  };
  /// \brief Default noexcept destructor
  ~AllocatableArray() noexcept = default;

  /// \brief Adds an item to this array
  /// If UniqueMembers is true, checks to see if an equivalent item 
  /// already exists, and applies the assignment operator to it.
  /// Returns false if full, rather than throwing an exception.
  bool add(ValBaseT toAdd) noexcept {
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

  // /**
  //  * @brief Helper to pass an initlist to the base value type
  //  * 
  //  * @tparam  
  //  * @param args The initialiser list arguments
  //  * @return true If the element is added
  //  * @return false If the element cannot be added (i.e. the array is full)
  //  */
  // template <typename... InitListArgsTs>
  // bool add(InitListArgsTs&... args) noexcept {
  //   return add(ValBaseT{args...});
  // }

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

  /// \brief Empties the container (lazy emptying)
  void clear() noexcept {
    m_allocated.reset();
  }

  /// \brief Returns the current assigned size of this array
  std::size_t size() const noexcept {
    return m_allocated.count();
  }

  /// \brief Returns the idx'th set value within the AllocatableArray
  value_type& operator[](std::size_t idx) noexcept {
    return m_members[realIndex(idx)];
  }

  /// \brief Returns the idx'th set value within the AllocatableArray as const
  constexpr const value_type& operator[](std::size_t idx) const noexcept {
    return m_members[realIndex(idx)];
  }

  /// \brief Support for const iterator begin
  const_iterator cbegin() const noexcept {
    return const_iterator(*this);
  }

  /// \brief Support for const iterator end
  const_iterator cend() const noexcept {
    if (size() == 0) return const_iterator(*this);
    return const_iterator(*this, size());
  }

  // /// \brief Support for const iterator begin
  // constexpr const_iterator begin() const noexcept {
  //   return const_iterator(*this);
  // }

  // /// \brief Support for const iterator end
  // constexpr const_iterator end() const noexcept {
  //   if (size() == 0) return const_iterator(*this);
  //   return const_iterator(*this, size());
  // }

  /// \brief Support for iterator begin
  iterator begin() noexcept {
    return iterator(*this);
  }

  /// \brief Support for iterator end
  iterator end() noexcept {
    if (size() == 0) return iterator(*this);
    return iterator(*this, size());
  }

  // TODO consider adding a find method

private:
  std::bitset<max_size> m_allocated;
  std::array<value_type,max_size> m_members;

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
        if (m_members[idx] == equiv) { // assumes value_type has operator==(EqualT)
        // if (equiv == (EqualT)m_members[idx]) {
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
        // Increment count
        ++count;
        // Now check if we're at the virtualIndex'th allocated element
        if (virtualIndex == count - 1) {
          // return this index
          return idx;
        }
        lastMatchedIndex = idx;
      }
      ++idx;
    }
    return lastMatchedIndex; // Always the last index that is set
  }
};

// template <typename... ValT, typename CommonValT = typename std::common_type_t<ValT...>, typename CommonValTValue = typename CommonValT::value_type>
// AllocatableArray(ValT... valueList) -> AllocatableArray<CommonValT,8,true>;


template <typename T, std::size_t Size = 8, bool UniqueMembers = true>
using ReferenceArray = AllocatableArray<std::optional<std::reference_wrapper<T>>,Size,UniqueMembers>;


/// \brief Provides a forward and reverse iterator for an AllocatableArray
/// \brief Implements noexcept guarantees
template <typename AllocatableArrayT,
          typename ValT, typename ValBaseT>
struct AllocatableArrayIterator {
  using difference_type = std::size_t;
  using value_type = ValBaseT;
  /// \brief Iterator Category expected by C++ stdlib
  using iterator_category = std::forward_iterator_tag;
  using pointer = value_type*;
  // using const_pointer = const value_type*;
  using reference = value_type&;
  // using const_reference = const value_type&;

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

  AllocatableArrayIterator& operator=(const AllocatableArrayIterator& other) noexcept {
    m_aa = other.m_aa;
    pos = other.pos;
    return *this;
  }

  /// \brief Dereference operator to return the value behind this iterator
  // template <typename Ref = reference, typename = std::enable_if_t<!std::is_same_v<reference,const_reference>> >
  reference operator*() noexcept {
    return m_aa[pos];
  }

  // constexpr const_reference operator*() const noexcept {
  //   return m_aa[pos];
  // }

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


  /// \brief Allows this iterator to be moved forward
  AllocatableArrayIterator<AllocatableArrayT>& operator+=(std::size_t by) noexcept {
    pos += by;
    if (pos > m_aa.size()) {
      pos = m_aa.size();
    }
    return *this;
  }

  /// \brief Allows this iterator to be moved backward
  AllocatableArrayIterator<AllocatableArrayT>& operator-=(std::size_t by) noexcept {
    if (by > pos) {
      pos = 0;
    } else {
      pos -= by;
    }
    return *this;
  }

  /// \brief Minus operator to allow std::distance to work
  difference_type operator-(const AllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos - other.pos;
  }

  bool operator<(const AllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos < other.pos;
  }

  bool operator>(const AllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos > other.pos;
  }

  /// \brief Prefix increment operator
  AllocatableArrayIterator<AllocatableArrayT>& operator++() noexcept {
    ++pos;
    return *this;
  }

  /// \brief Postfix increment operator
  AllocatableArrayIterator<AllocatableArrayT>& operator++(int) noexcept {
    AllocatableArrayIterator<AllocatableArrayT> cp = *this;
    ++pos;
    return cp;
  }

  /// \brief Prefix decrement operator
  AllocatableArrayIterator<AllocatableArrayT>& operator--() noexcept {
    --pos;
    return *this;
  }

  /// \brief Postfix decrement operator
  AllocatableArrayIterator<AllocatableArrayT>& operator--(int) noexcept {
    AllocatableArrayIterator<AllocatableArrayT> cp = *this;
    --pos;
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













/// \brief Provides a forward and reverse iterator for an AllocatableArray
/// \brief Implements noexcept guarantees
template <typename AllocatableArrayT,
          typename ValT, typename ValBaseT>
struct ConstAllocatableArrayIterator {
  using difference_type = std::size_t;
  using value_type = const ValBaseT;
  /// \brief Iterator Category expected by C++ stdlib
  using iterator_category = std::forward_iterator_tag;
  using pointer = const value_type*;
  // using const_pointer = const value_type*;
  using reference = const value_type&;
  // using const_reference = const value_type&;

  /// \brief Iterator constructor starting at virtual index 0
  ConstAllocatableArrayIterator(const AllocatableArrayT& aa) noexcept : m_aa(aa), pos(0) {}
  /// \brief Iterator constructor starting at virtual index from
  ConstAllocatableArrayIterator(const AllocatableArrayT& aa, std::size_t from) noexcept : m_aa(aa), pos(from) {}
  /// \brief Copy constructor
  ConstAllocatableArrayIterator(const ConstAllocatableArrayIterator& other) noexcept : m_aa(other.m_aa), pos(other.pos) {}
  /// \brief Move constructor
  ConstAllocatableArrayIterator(ConstAllocatableArrayIterator&& other) noexcept : m_aa(other.m_aa), pos(other.pos) {}
  /// \brief Default destructor
  ~ConstAllocatableArrayIterator() noexcept = default;

  ConstAllocatableArrayIterator& operator=(const ConstAllocatableArrayIterator& other) noexcept {
    m_aa = other.m_aa;
    pos = other.pos;
    return *this;
  }

  /// \brief Dereference operator to return the value behind this iterator
  // template <typename Ref = reference, typename = std::enable_if_t<!std::is_same_v<reference,const_reference>> >
  reference operator*() const noexcept {
    return m_aa[pos];
  }

  pointer operator->() const noexcept {
    return &m_aa[pos];
  }

  /// \brief Allows this iterator to be moved forward
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator+(std::size_t by) noexcept {
    pos += by;
    if (pos > m_aa.size()) {
      pos = m_aa.size();
    }
    return *this;
  }

  /// \brief Allows this iterator to be moved backward
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator-(std::size_t by) noexcept {
    if (by > pos) {
      pos = 0;
    } else {
      pos -= by;
    }
    return *this;
  }


  /// \brief Allows this iterator to be moved forward
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator+=(std::size_t by) noexcept {
    pos += by;
    if (pos > m_aa.size()) {
      pos = m_aa.size();
    }
    return *this;
  }

  /// \brief Allows this iterator to be moved backward
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator-=(std::size_t by) noexcept {
    if (by > pos) {
      pos = 0;
    } else {
      pos -= by;
    }
    return *this;
  }

  /// \brief Minus operator to allow std::distance to work
  difference_type operator-(const ConstAllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos - other.pos;
  }

  bool operator<(const ConstAllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos < other.pos;
  }

  bool operator>(const ConstAllocatableArrayIterator<AllocatableArrayT>& other) const noexcept {
    return pos > other.pos;
  }

  /// \brief Prefix increment operator
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator++() noexcept {
    ++pos;
    return *this;
  }

  /// \brief Postfix increment operator
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator++(int) noexcept {
    ConstAllocatableArrayIterator<AllocatableArrayT> cp = *this;
    ++pos;
    return cp;
  }

  /// \brief Prefix decrement operator
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator--() noexcept {
    --pos;
    return *this;
  }

  /// \brief Postfix decrement operator
  ConstAllocatableArrayIterator<AllocatableArrayT>& operator--(int) noexcept {
    AllocatableArrayIterator<AllocatableArrayT> cp = *this;
    --pos;
    return cp;
  }

  /// \brief Equality operator. Compares position.
  bool operator==(const ConstAllocatableArrayIterator<AllocatableArrayT>& otherIter) const noexcept {
    return pos == otherIter.pos;
  }

  /// \brief Inequality operator. Compares position.
  bool operator!=(const ConstAllocatableArrayIterator<AllocatableArrayT>& otherIter) const noexcept {
    return pos != otherIter.pos;
  }

private:
  const AllocatableArrayT& m_aa;
  std::size_t pos;
};

/// \brief Distance operator for std::distance
template <typename T>
typename ConstAllocatableArrayIterator<T>::difference_type distance(ConstAllocatableArrayIterator<T> first, ConstAllocatableArrayIterator<T> last) noexcept {
  return last - first;
}














/**
 * /brief An AllocatableArray where each value is linked to a metadata Tag instance
 */
template <typename TagT, typename ValT, 
          std::size_t Size=8, bool UniqueMembers=false,
          typename ValBaseT = typename std::remove_cv<ValT>::type,
          typename AAT=AllocatableArray<ValBaseT,Size,UniqueMembers>
         >
struct TaggedArray {
  using value_type = typename AAT::value_type;
  static constexpr std::size_t max_size = AAT::max_size;

  TaggedArray() noexcept
   : tag(),
     aa()
  {
    ;
  }

  TaggedArray(const TagT& arrayTag) noexcept
   : tag(arrayTag),
     aa()
  {
    ;
  }

  ~TaggedArray() noexcept = default;

  const TagT& getTag() const noexcept
  {
    return tag;
  }

  void setTag(const TagT& newValue) noexcept
  {
    tag = newValue;
  }

  const bool operator==(const TaggedArray& other) const noexcept
  {
    return tag == other.tag;
  }

  const bool operator!=(const TaggedArray& other) const noexcept
  {
    return tag != other.tag;
  }

  const bool operator==(const TagT& other) const noexcept
  {
    return tag == other;
  }

  const bool operator!=(const TagT& other) const noexcept
  {
    return tag != other;
  }

  const AAT& ccontents() const noexcept
  {
    return aa;
  }

  AAT& contents() noexcept
  {
    return aa;
  }

private:
  TagT tag;
  AAT aa;
};



template <typename TagT, typename ValT,
          std::size_t TaggedArraySize=8, bool TaggedArrayUniqueMembers=false,
          std::size_t SetSize=8, bool SetUniqueMembers=true,
          typename ValBaseT = typename std::remove_cv<ValT>::type
         >
using TaggedArraySet = AllocatableArray<
  TaggedArray<TagT,ValBaseT,TaggedArraySize,TaggedArrayUniqueMembers>,
  SetSize,SetUniqueMembers
>;


/**
 * @brief Represents a value within an ArrayMap
 * 
 * @tparam TagT The Tag type (same for all elements of the parent ArrayMap)
 * @tparam ValT The value type (same for all elements of the parent ArrayMap)
 * @tparam std::remove_cv<ValT>::type The base type without const or volatile
 */
template <typename TagT, typename ValT, typename ValBaseT = typename std::remove_cv<ValT>::type>
struct ArrayMapPair {
  TagT key;
  ValBaseT value;

  bool operator==(const ArrayMapPair& other) const noexcept {
    return key == other.key;
  }
  
  bool operator!=(const ArrayMapPair& other) const noexcept {
    return key != other.key;
  }
};


/**
 * /brief An AllocatableArray where each value is linked to a metadata Tag instance
 */
template <typename TagT, typename ValT, 
          std::size_t Size=8, bool UniqueMembers=true,
          typename ValBaseT = typename std::remove_cv<ValT>::type,
          typename AAT=AllocatableArray<ArrayMapPair<TagT,ValBaseT>,Size,UniqueMembers>
         >
struct ArrayMap {
  using value_type = ValBaseT;
  static constexpr std::size_t max_size = AAT::max_size;

  ArrayMap() noexcept
   : aa()
  {
    ;
  }

  ~ArrayMap() noexcept = default;

  bool get(const TagT& tag, ValBaseT& toSet) const noexcept {
    std::size_t pos = tagPosition(tag);
    if (AAT::max_size == pos) {
      return false;
    }
    toSet = aa[pos].value;
    return true;
  }

  bool set(const TagT& tag, const ValT& value) noexcept {
    std::size_t pos = tagPosition(tag);
    if (AAT::max_size == pos) {
      if (aa.size() == AAT::max_size) {
        return false;
      }
      // TODO determine if we even need to do this logic, given UniqueMemebers = true
      aa.add(ArrayMapPair<TagT,ValBaseT>{.key = tag, .value = value});
    } else {
      aa[pos].value = value;
    }
    return true;
  }

  std::size_t size() const noexcept {
    return aa.size();
  }

  bool hasTag(const TagT& tag) const noexcept {
    return AAT::max_size != tagPosition(tag);
  }

private:
  AAT aa;

  std::size_t tagPosition(const TagT& tag) const noexcept {
    for (std::size_t pos = 0;pos < aa.size();++pos) {
      if (aa[pos].key == tag) {
        return pos;
      }
    }
    return AAT::max_size;
  }
};



}
}

namespace {

}

namespace std {

/// \brief Less than operator overload for std::optional
template <typename T>
std::size_t operator<(const std::optional<std::reference_wrapper<T>>& lhs,const std::optional<std::reference_wrapper<T>>& rhs) noexcept {
  if (lhs.has_value() && !rhs.has_value()) {
    return 1;
  }
  if (rhs.has_value() && !lhs.has_value()) {
    return 0;
  }
  return lhs.value().get() < rhs.value().get();
}

template <typename T>
bool operator==(const std::optional<std::reference_wrapper<T>>& lhs,const std::optional<std::reference_wrapper<T>>& rhs) noexcept {
  if (lhs.has_value() && !rhs.has_value()) {
    return false;
  }
  if (rhs.has_value() && !lhs.has_value()) {
    return false;
  }
  // Check for presence of operator== on base type T
  if constexpr (herald::util::HasEqualityFunctionV<decltype(lhs),decltype(lhs),decltype(rhs)>) {
    return lhs.value().get() == rhs.value().get();
  } else {
    return (&lhs) == (&rhs);
  }
}

template <typename T>
bool operator!=(const std::optional<std::reference_wrapper<T>>& lhs,const std::optional<std::reference_wrapper<T>>& rhs) noexcept {
  if (lhs.has_value() && !rhs.has_value()) {
    return true;
  }
  if (rhs.has_value() && !lhs.has_value()) {
    return true;
  }
  // Check for presence of operator!= on base type T
  if constexpr (herald::util::HasInequalityFunctionV<decltype(lhs),decltype(lhs),decltype(rhs)>) {
    return lhs.value().get() != rhs.value().get();
  } else {
    return (&lhs) != (&rhs);
  }
}

}

#endif