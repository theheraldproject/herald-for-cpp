//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_RANGES_H
#define HERALD_RANGES_H

// TODO if def for C++20 support check (so we don't have to roll our own ranges lib)
// i.e. map std::views on to herald::analysis::views, otherwise include the following.

/// We may not use C++20 or similar ranges - we need to be super memory efficient in
/// our implementation for embedded devices, and we cannot use compiler features not
/// yet present in gcc-arm.

#include <array>
#include <map>
#include <variant>
#include <vector>
#include <cstdint>
#include <type_traits>

#include "sampling.h"
#include "../datatype/date.h"

namespace herald {
namespace analysis {
namespace views {

// Note: The following are SAMPLE filters, and only work with Samples
struct since {
  since(herald::datatype::Date after) : from(after) {}
  ~since() = default;

  template <typename ValT>
  bool operator()(const herald::analysis::sampling::Sample<ValT>& s) const {
    return s.taken > from;
  }

private:
  herald::datatype::Date from;
};

struct beforeOrEqual {
  beforeOrEqual(herald::datatype::Date timeToInclusive) : to(timeToInclusive) {}
  ~beforeOrEqual() = default;

  template <typename ValT>
  bool operator()(const herald::analysis::sampling::Sample<ValT>& s) const {
    return s.taken <= to;
  }

private:
  herald::datatype::Date to;
};

// Note: The following are value filters, and work with Samples and any other type

/// dual or chained filter
template <typename Pred1,typename Pred2>
struct dual_filter {
  dual_filter(const Pred1 p1, const Pred2 p2) : pred1(p1), pred2(p2) {}
  ~dual_filter() = default;

  template <typename VTOther>
  bool operator()(const VTOther& value) const {
    return pred1(value) && pred2(value);
  }

private:
  const Pred1 pred1;
  const Pred2 pred2;
};

// Is genericised to...
template <typename VT>
struct in_range {
  in_range(const VT min, const VT max) : min(min), max(max) {}
  ~in_range() = default;

  template <typename VTOther>
  bool operator()(const VTOther& value) const {
    return value >= min && value <= max;
  }

private:
  const VT min;
  const VT max;
};

template <typename VT>
struct greater_than {
  greater_than(const VT min) : min(min) {}
  ~greater_than() = default;

  template <typename VTOther>
  bool operator()(const VTOther& value) const {
    return value > min;
  }

private:
  const VT min;
};

template <typename VT>
struct less_than {
  less_than(const VT max) : max(max) {}
  ~less_than() = default;

  template <typename VTOther>
  bool operator()(const VTOther& value) const {
    return value < max;
  }

private:
  const VT max;
};

// TODO consider clamping modifications as well as in_range filtering

/// Proxies a collection's iterator
/// In this implementation, does nothing else
/// Use like:-
/// iterator_proxy<> proxy(myCollection);
/// while (proxy != std::end(myCollection)) { // Could also use !proxy.ended()
///   std::cout << *proxy << std::endl; // prints ALL numbers
///   ++proxy;
/// }
template <typename Coll,
          //typename ValT = typename std::remove_cv<typename Coll::value_type::first_type>::type,
          typename ValT = typename std::remove_cv<typename Coll::value_type>::type, // works for intrinsic types and complex types
          typename IterT = typename Coll::iterator,
          typename SizeT = typename Coll::size_type
         >
struct iterator_proxy {
  using base_iterator = IterT;
  using base_value_type = ValT;
  using base_size_type = SizeT;

  iterator_proxy(Coll& coll) : coll(coll), iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))) {}
  iterator_proxy(iterator_proxy&& other) : coll(other.coll), iter(std::move(other.iter)), endIter(std::move(other.endIter)) {}
  iterator_proxy(const iterator_proxy& other) : coll(other.coll), iter(other.iter), endIter(other.endIter) {}
  ~iterator_proxy() = default;

  auto operator*() -> const ValT& {
    return *iter;
  }

  /// prefix operator
  iterator_proxy<Coll>& operator++() {
    ++iter;
    return *this; // reference to instance
  }

  // postfix operator
  iterator_proxy<Coll> operator++(int) {
    iterator_proxy<Coll> cp =  *this; // copy of instance
    ++(*this);
    return cp;
  }

  // Increment operator
  iterator_proxy<Coll>& operator+=(int by) {
    iter += by;
    return *this; // reference to instance
  }

  // Decrement operator
  iterator_proxy<Coll>& operator-=(int by) {
    iter -= by;
    return *this; // reference to instance
  }

  bool operator==(IterT otherIter) const {
    return iter == otherIter;
  }

  bool operator!=(IterT otherIter) const {
    return iter != otherIter;
  }

  friend bool operator!=(IterT otherIter,iterator_proxy<Coll> thisIter) {
    return otherIter != thisIter.iter;
  }

  friend bool operator==(IterT otherIter,iterator_proxy<Coll> thisIter) {
    return otherIter == thisIter.iter;
  }

  IterT& wrapped() {
    return iter;
  }

  IterT& end() {
    return endIter;
  }

  bool ended() {
    return endIter == iter;
  }

  Coll& collection() const {
    return coll;
  }

private:
  Coll& coll;
  IterT iter;
  IterT endIter;
};

///
/// Use like 
/// filter_fn<> myFilter(in_range<int>(18,65));
/// bool passes = myFilter(45);
///
template <typename Pred>
struct filter_fn {
  filter_fn(const Pred pred) : pred(pred) {}
  ~filter_fn() = default;

  template <typename ValT>
  auto operator()(const ValT& val) -> bool {
    return pred(val);
  }

  const Pred predicate() const {
    return pred;
  }

private:
  const Pred pred;
};

/// Create a view holder that wraps an iterator, so the result of all returns have a begin() and end()
/// just like an STL collection
template <typename IterProxyT,
          typename BaseValT = typename IterProxyT::base_value_type,
          typename BaseIterT = typename IterProxyT::base_iterator,
          typename BaseSizeT = typename IterProxyT::base_size_type>
struct view {
  // Make this look like an STL collection (so filter<Coll,Pred> works with a view)
  using value_type = BaseValT;
  using iterator = BaseIterT;
  using size_type = BaseSizeT;

  using is_proxy = std::true_type;

  view(IterProxyT srcIter) : source(std::forward<IterProxyT>(srcIter)) {} // TAKE OWNERSHIP
  ~view() = default;

  auto begin() -> IterProxyT {
    return source;
  }

  auto end() -> BaseIterT {
    return source.end();
  }

  // bool hasData() noexcept {
  //   return source != source.end();
  // }

  // auto latest() -> BaseValT {
  //   //return source.latest();
  //   return *(source.end() - 1);
  // }
  Date latest() {
    return (*(source.end() - 1)).taken;
  }

  Date earliest() {
    return (*(source.begin())).taken;
  }

  template <typename IterT>
  bool operator==(const IterT& other) {
    return other == source;
  }

  template <typename IterT>
  bool operator!=(const IterT& other) {
    return other != source;
  }

  auto size() -> BaseSizeT {
    // return source.size(); // this is the UNFILTERED size
    BaseSizeT sz = 0;
    auto iter = source; // copy the iterator so as not to alter its state
    auto end = source.end(); // copy the iterator so as not to alter its state
    while (iter != end) {
      ++sz;
      ++iter;
    }
    // minus one as we don't want the distance to end() but the last element the 1 before it
    return sz;
    // return std::distance(source.wrapped(),source.end() - 1);
  }

  auto operator[](BaseSizeT position) -> BaseValT {
    return *(source + position);
  }

private:
  IterProxyT source;
};

/// Now we create a version that selectively returns the underlying iterator's elements using filter_fn
/// Use like:-
/// myCollection is a collection with 14,16,19,45,66 in it (ints)
/// filtered_iterator_proxy<> proxy(myCollection, in_range<int>(18,65));
/// while (proxy != std::end(myCollection)) { // Could also use !proxy.ended()
///   std::cout << *proxy << std::endl; // prints ONLY 19 and 45
///   ++proxy;
/// }
template <typename Coll,
          typename Pred,
          // typename ValT = typename std::remove_cv<typename Coll::value_type::first_type>::type,
          typename ValT = typename std::remove_cv<typename Coll::value_type>::type, // works for intrinsic types and complex types
          typename IterT = typename Coll::iterator,
          typename SizeT = typename Coll::size_type
         >
struct filtered_iterator_proxy {
  using base_iterator = IterT;
  using base_coll_type = Coll; // for chaining
  using base_pred_type = Pred; // for chaining
  using base_value_type = ValT;
  using base_size_type = SizeT;

  using value_type = ValT;
  using iterator = IterT;
  using size_type = SizeT;
  // using difference_type = typename Coll::difference_type;

  using is_proxy = std::true_type;

  filtered_iterator_proxy(Coll& coll, Pred pred) : coll(coll), iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))), filter(pred) {
    // move forward to the first match (or end)
    moveToFirst();
  }

  // chaining ctor
  // filtered_iterator_proxy(filtered_iterator_proxy&& other, Pred pred) : iter(std::move(other.iter)), endIter(std::move(other.endIter)), filter(dual_filter(other.filter,pred)) {}
  filtered_iterator_proxy(filtered_iterator_proxy&& other) : coll(other.coll), iter(std::move(other.iter)), endIter(std::move(other.endIter)), filter(other.filter) {}
  filtered_iterator_proxy(const filtered_iterator_proxy& other) : coll(other.coll), iter(other.iter), endIter(other.endIter), filter(other.filter) {}
  ~filtered_iterator_proxy() = default;

  auto operator*() -> const ValT& {
    return *iter;
  }

  /// prefix operator
  filtered_iterator_proxy<Coll,Pred>& operator++() {
    // move forward until we get a match (or end)
    moveToNext();
    return *this;
  }

  // postfix operator
  filtered_iterator_proxy<Coll,Pred> operator++(int) {
    filtered_iterator_proxy<Coll,Pred> cp =  *this; // copy of instance
    ++(*this);
    return cp;
  }

  filtered_iterator_proxy<Coll,Pred> operator+(int by) {
    filtered_iterator_proxy<Coll,Pred> cp =  *this; // copy of instance
    for (int i = 0;i < by;++i) {
      ++cp;
    }
    return cp;
  }

  bool operator==(IterT otherIter) const {
    return iter == otherIter;
  }

  bool operator!=(IterT otherIter) const {
    return iter != otherIter;
  }

  friend bool operator!=(IterT otherIter,filtered_iterator_proxy<Coll,Pred> thisIter) {
    return otherIter != thisIter.iter;
  }

  friend bool operator==(IterT otherIter,filtered_iterator_proxy<Coll,Pred> thisIter) {
    return otherIter == thisIter.iter;
  }

  IterT& wrapped() {
    return iter;
  }

  IterT& end() {
    return endIter;
  }

  bool ended() {
    return endIter == iter;
  }

  Coll& collection() const {
    return coll;
  }

  Pred predicate() const {
    return filter.predicate();
  }

private:
  Coll& coll;
  IterT iter;
  IterT endIter;
  filter_fn<Pred> filter;

  // Need this function so as not to incorrectly always filter the first value in the underlying proxy
  void moveToFirst() {
    while (endIter != iter && !filter(*iter)) {
      ++iter;
    };
  }

  void moveToNext() {
    if (endIter == iter) return; // guard
    do {
      ++iter;
    } while (endIter != iter && !filter(*iter));
  }
};

/// Now think about a filter instance that would filter an entire range of values
///
/// Use like
/// filter<> myFilter(in_range<int>(18,65));
/// auto iterable = myFilter(myCollection);
///
template <typename Pred>
struct filter {
public:
  filter(const Pred& pred) : pred(pred) {}
  ~filter() = default;

  template <typename Coll>
  auto operator()(Coll& c) -> filtered_iterator_proxy<Coll,Pred> {
    return filtered_iterator_proxy<Coll,Pred>(c,pred);
  }
  
  template <typename OtherColl, typename OtherPred> // first argument must be l-value below
  friend auto operator|(filtered_iterator_proxy<OtherColl,OtherPred> c,filter<Pred> pred) -> filtered_iterator_proxy<OtherColl,dual_filter<OtherPred,Pred>> {
    return filtered_iterator_proxy<OtherColl,dual_filter<OtherPred,Pred>>(c.collection(),dual_filter(c.predicate(),pred.pred));
  }

  template <typename Coll>
  friend auto operator|(Coll& c,filter<Pred> pred) -> filtered_iterator_proxy<Coll,Pred> {
    return filtered_iterator_proxy<Coll,Pred>(c,pred.pred);
  }

private:
  Pred pred;
};

/// Simple action to convert the end of a *iterator_proxy chain in to a final view class
/// The final view class has begin() and end() and other STL collection like features
struct to_view {
  to_view() = default;
  ~to_view() = default;

  // convert something with a proxy (likely a filtered iterator proxy) into a view
  template <typename IterProxyT>
  friend auto operator|(IterProxyT proxy,to_view view) -> herald::analysis::views::view<IterProxyT> {
    return herald::analysis::views::view<IterProxyT>(proxy);
  }
  
  /// Convert an unfiltered source collection into a view
  // template <typename Coll,
  //           typename IterProxyT = iterator_proxy<Coll>, 
  //           std::enable_if_t<!std::is_same_v<Coll,iterator_proxy> && !std::is_same_v<Coll,filtered_iterator_proxy>, bool> = true
  //          >
  // friend auto operator|(Coll coll,to_view view) -> herald::analysis::views::view<IterProxyT> {
  //   return herald::analysis::views::view<IterProxyT>(iterator_proxy<Coll>(coll));
  // }
  
  template <typename SampleT,
            std::size_t ListSize,
            typename IterProxyT = iterator_proxy<herald::analysis::sampling::SampleList<SampleT,ListSize>>
           >
  friend auto operator|(herald::analysis::sampling::SampleList<SampleT,ListSize>& coll,to_view view) -> herald::analysis::views::view<IterProxyT> {
    return herald::analysis::views::view<IterProxyT>(iterator_proxy<herald::analysis::sampling::SampleList<SampleT,ListSize>>(coll));
  }
};

} // end namespace views
}
}

#endif
