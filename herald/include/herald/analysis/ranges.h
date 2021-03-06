//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef RANGES_H
#define RANGES_H

// TODO if def for C++20 support check (so we don't have to roll our own ranges lib)
// i.e. map std::views on to herald::analysis::views, otherwise include the following.

/// We may not use C++20 or similar ranges - we need to be super memory efficient in
/// our implementation for embedded devices, and we cannot use compiler features not
/// yet present in gcc-arm.

#include <array>
#include <cstdint>

namespace herald {
namespace analysis {

/// A set of structs compatible with, but not reliant upon, views and ranges in Herald
namespace sampling {

template <typename ValT>
struct Sample {
  using value_type = ValT;

  Date taken; // Date first for alignment reasons
  ValT value;

  Sample() : taken(), value() {} // default ctor (required for array)
  Sample(Date sampled, ValT v) : taken(sampled), value(v) {}
  ~Sample() = default;
};

/// FWD DECL
// template <typename ValT, 
//           std::size_t MaxSize>
template <typename SampleListT,
          typename ValT = typename SampleListT::value_type>
struct SampleIterator;

/// A Circular container for Samples
/// Can be used as a container in the views library
template <typename SampleT, // This is Sample<SampleValueT>
          std::size_t MaxSize,
          typename SampleValueT = typename std::remove_cv<typename SampleT::value_type>::type
         >
struct SampleList {
  using value_type = SampleT; // MUST be before the next line!
  using iterator = SampleIterator<SampleList<SampleT,MaxSize>>;
  using size_type = std::size_t;

  static constexpr std::size_t max_size = MaxSize;

  SampleList() : data(), oldestPosition(SIZE_MAX), newestPosition(SIZE_MAX) {};
  ~SampleList() = default;

  void push(Date taken, SampleValueT val) {
    if (SIZE_MAX == newestPosition) {
      newestPosition = 0;
      oldestPosition = 0;
    } else {
      if (newestPosition == (oldestPosition - 1)) {
        ++oldestPosition;
        if (oldestPosition == data.size()) {
          oldestPosition = 0;
        }
      }
      ++newestPosition;
    }
    if (newestPosition == data.size()) {
      // just gone past the end of the container
      newestPosition = 0;
      if (0 == oldestPosition) {
        ++oldestPosition; // erases oldest if not already removed
      }
    }
    data[newestPosition] = SampleT{taken,val};
  }

  std::size_t size() const {
    if (newestPosition == SIZE_MAX) return 0;
    if (newestPosition >= oldestPosition) {
      // not overlapping the end
      return newestPosition - oldestPosition + 1;
    }
    // we've overlapped
    return (1 + newestPosition) + (data.size() - oldestPosition);
  }

  const SampleT& operator[](std::size_t idx) const {
    if (newestPosition >= oldestPosition) {
      return data[idx + oldestPosition];
    }
    if (idx + oldestPosition >= data.size()) {
      // TODO handle the situation where this pos > newestPosition (i.e. gap in the middle)
      return data[idx + oldestPosition - data.size()];
    }
    return data[idx + oldestPosition];
  }

  void clearBeforeDate(const Date& before) {
    if (SIZE_MAX == oldestPosition) return;
    while (oldestPosition != newestPosition) {
      if (data[oldestPosition].taken < before) {
        ++oldestPosition;
        if (data.size() == oldestPosition) {
          // overflowed
          oldestPosition = 0;
        }
      } else {
        return;
      }
    }
    // now we're on the last element
    if (data[oldestPosition].taken < before) {
      // remove last element
      oldestPosition = SIZE_MAX;
      newestPosition = SIZE_MAX;
    }
  }

  void clear() {
    oldestPosition = SIZE_MAX;
    newestPosition = SIZE_MAX;
  }

  SampleIterator<SampleList<SampleT,MaxSize>> begin() {
    return SampleIterator<SampleList<SampleT,MaxSize>>(*this);
  }

  SampleIterator<SampleList<SampleT,MaxSize>> end() {
    if (size() == 0) return SampleIterator<SampleList<SampleT,MaxSize>>(*this);
    return SampleIterator<SampleList<SampleT,MaxSize>>(*this,size()); // calls this object's size() function, not the array!
  }

private:
  std::array<SampleT,MaxSize> data;
  std::size_t oldestPosition;
  std::size_t newestPosition;
};

template <typename SampleListT,
          typename ValT> // from fwd decl =>  = typename SampleListT::value_type
struct SampleIterator {
  SampleIterator(SampleListT& sl) : list(sl), pos(0) {}
  SampleIterator(SampleListT& sl, std::size_t from) : list(sl), pos(from) {} // used to get list.end() (size() + 1)
  SampleIterator(const SampleIterator<SampleListT>& other) : list(other.list), pos(other.pos) {} // copy ctor
  SampleIterator(SampleIterator<SampleListT>&& other) : list(other.list), pos(other.pos) {} // move ctor (cheaper to copy)
  ~SampleIterator() = default;

  // always returns const for safety
  const ValT& operator*() {
    return list[pos];
  }

  // TODO implement operator+(int amt) to move this iterator forward

  /// prefix operator
  SampleIterator<SampleListT>& operator++() {
    ++pos; // if it's one after the end of the list, then that's the same as list.end()
    return *this; // reference to instance
  }

  // postfix operator
  SampleIterator<SampleListT> operator++(int) {
    SampleIterator<SampleListT> cp =  *this; // copy of instance
    ++(*this);
    return cp;
  }

  bool operator==(const SampleIterator<SampleListT>& otherIter) const {
    return pos == otherIter.pos;
  }

  bool operator!=(const SampleIterator<SampleListT>& otherIter) const {
    return pos != otherIter.pos;
  }

  // friend bool operator!=(SampleIterator<ValT,MaxSize> otherIter,SampleIterator<ValT,MaxSize> thisIter) {
  //   return otherIter != thisIter;
  // }

  // friend bool operator==(SampleIterator<ValT,MaxSize> otherIter,SampleIterator<ValT,MaxSize> thisIter) {
  //   return otherIter == thisIter;
  // }


private:
  SampleListT& list;
  std::size_t pos;
};

} // end sampling namespace

namespace views {

// TEMPORARY thought exercise - an example Predicate for a specific type
/*
struct in_range_int {
public:
  in_range_int(const int min, const int max) : min(min), max(max) {}
  ~in_range_int() = default;

  template <typename VT>
  bool operator()(const VT& value) const {
    return value.value() >= min && value().value() <= max;
  }

private:
  int min;
  int max;
};
*/

/// dual or chained filter
template <typename Pred1,typename Pred2>
struct dual_filter {
public:
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
public:
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
public:
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
public:
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

  iterator_proxy(Coll& coll) : iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))) {}
  iterator_proxy(iterator_proxy&& other) : iter(std::move(other.iter)), endIter(std::move(other.endIter)) {}
  iterator_proxy(const iterator_proxy& other) : iter(other.iter), endIter(other.endIter) {}
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

private:
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

  view(IterProxyT srcIter) : source(std::forward<IterProxyT>(srcIter)) {} // TAKE OWNERSHIP
  ~view() = default;

  auto begin() -> IterProxyT {
    return source;
  }

  auto end() -> BaseIterT {
    return source.end();
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
    return std::distance(source.wrapped(),source.end() - 1); // minus one as we don't want the distance to end() but the last element the 1 before it
  }

  auto operator[](BaseSizeT position) -> BaseValT {
    return *(source.wrapped() + position);
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

  filtered_iterator_proxy(Coll& coll, Pred pred) : coll(coll), iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))), filter(pred) {
    // move forward to the first match (or end)
    moveToNext();
  }
  // // and from an intermediary proxy
  // template <typename OtherColl, typename OtherPred>
  // filtered_iterator_proxy(filtered_iterator_proxy<OtherColl,OtherPred>& otherProxy, Pred pred) 
  //   : iter(otherProxy), endIter(otherProxy.end()), filter(pred) {
  //   // move forward to the first match (or end)
  //   moveToNext();
  // }

  // chaining ctor
  // filtered_iterator_proxy(filtered_iterator_proxy&& other, Pred pred) : iter(std::move(other.iter)), endIter(std::move(other.endIter)), filter(dual_filter(other.filter,pred)) {}
  filtered_iterator_proxy(filtered_iterator_proxy&& other) : coll(other.coll), iter(std::move(other.iter)), endIter(std::move(other.endIter)), filter(other.filter) {}
  filtered_iterator_proxy(const filtered_iterator_proxy& other) : coll(other.coll), iter(other.iter), endIter(other.endIter), filter(other.filter) {}
  ~filtered_iterator_proxy() = default;

  auto operator*() -> ValT& {
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

  // template <typename Coll>
  // Coll& operator=(filtered_iterator_proxy<Coll,Pred> proxy) {
  //   // Iterate over iterator proxy and add each value in to the result coll
  //   // In effect, this performs filtering 'on the fly' when an = operator is encountered
  //   //   i.e. this is the point the chain of proxy iterators is evaluated - resulting in
  //   //   an iteration over the source collection just ONCE, and no temporary collections.
  //   // Note: Using STL functions for this (using resultColl, proxy.iter and proxy.end)
  //   resultColl.insert(resultColl.end(), proxy, proxy.end());
  //   return resultColl;
  // }

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

  // template <typename RangeIterator>
  // auto operator()(RangeIterator ri) -> RangeIterator {
  //   while (std::end() != ri) {
  //     if (pred(*ri)) {
  //       return ri;
  //     }
  //     ri++;
  //   }
  //   return ri; // is equal to the correctly typed std::end here
  // }

  template <typename Coll>
  auto operator()(Coll& c) -> filtered_iterator_proxy<Coll,Pred> {
    return filtered_iterator_proxy<Coll,Pred>(c,pred);
  }
  
  // specialisation for two chained filters
  // template <typename OtherPred>
  // friend auto operator|(filter<OtherPred> otherFilter,filter<Pred> pred) -> filtered_iterator_proxy<view<filter<OtherPred>>,Pred> {
  //   return filtered_iterator_proxy<view<filter<OtherPred>>,Pred>(view(otherFilter),pred.pred);
  // }
  // template <typename OtherColl, typename OtherPred>
  // friend auto operator|(filtered_iterator_proxy<OtherColl,OtherPred> otherProxy,filter<Pred> pred) -> filtered_iterator_proxy<OtherColl,Pred> {
  //   return filtered_iterator_proxy<OtherColl,Pred>(otherProxy,pred.pred);
  // }

  // template <typename OtherIterProxyT>
  // friend auto operator|(filter<OtherIterProxyT> c,filter<Pred> pred) -> filtered_iterator_proxy<view<OtherIterProxyT>,Pred> {
  //   return filtered_iterator_proxy<view<OtherIterProxyT>,Pred>(std::forward<view<OtherIterProxyT>>(c),pred.pred); // perfect forwarding
  // }
  template <typename OtherColl, typename OtherPred> // first argument must be l-value below
  friend auto operator|(filtered_iterator_proxy<OtherColl,OtherPred> c,filter<Pred> pred) -> filtered_iterator_proxy<OtherColl,dual_filter<OtherPred,Pred>> {
    return filtered_iterator_proxy<OtherColl,dual_filter<OtherPred,Pred>>(c.collection(),dual_filter(c.predicate(),pred.pred));
  }

  // template <typename ValT, std::size_t MaxSize> // first argument must be l-value below
  // friend auto operator|(herald::analysis::sampling::SampleList<ValT,MaxSize>& c,filter<Pred> pred) -> filtered_iterator_proxy<herald::analysis::sampling::SampleList<ValT,MaxSize>,Pred> {
  //   return filtered_iterator_proxy<herald::analysis::sampling::SampleList<ValT,MaxSize>,Pred>(c,pred.pred);
  // }

  template <typename Coll>
  friend auto operator|(Coll& c,filter<Pred> pred) -> filtered_iterator_proxy<Coll,Pred> {
    return filtered_iterator_proxy<Coll,Pred>(c,pred.pred);
  }

  // template <typename Coll>
  // friend auto operator|(Coll c,filter<Pred> pred) -> view<filtered_iterator_proxy<Coll,Pred>> {
  //   return view<filtered_iterator_proxy<Coll,Pred>>(filtered_iterator_proxy<Coll,Pred>(c,pred.pred));
  // }

private:
  Pred pred;
};

/// Simple action to convert the end of a *iterator_proxy chain in to a final view class
/// The final view class has begin() and end() and other STL collection like features
struct to_view {
  to_view() = default;
  ~to_view() = default;

  template <typename IterProxyT>
  friend auto operator|(IterProxyT proxy,to_view view) -> herald::analysis::views::view<IterProxyT> {
    return herald::analysis::views::view<IterProxyT>(proxy);
  }
};

/*
// FILTER - FUNCTION - Takes a Range (via |) and a predicate(via ()), produces a view that filters as it goes
template <typename Func>
struct view {
  view(Func f) : func(f) {}
  ~view() = default;

  template <typename Rng>
  friend auto operator|(Rng range, view<Func> vw) -> {
    auto rngIter = range.cbegin();
    while
  }

private:
  Func func;
};
*/



/*
template <typename Pred>
struct filter {
public:
  filter(const Pred& pred) : mPred(pred) {}
  ~filter() = default;

  template <typename VT>
  bool operator()(const VT& value) {
    return mPred(value); // calls predicate's call operator() -> bool
  }

private:
  Pred mPred;
};
*/



/*
struct filter_fn {
public:
  filter_fn() = default;
  ~filter_fn() = default;

  template <typename Rng, typename Pred>
  auto operator()(const Rng& inRange, const Pred& ff) const -> Rng
  {
    return RangeIterator<Rng,Pred>(inRange,Pred);
  }

  template <typename Rng, typename Pred>
  friend constexpr auto operator|(const Rng& inRange, const Pred& pred) -> Rng
  {

  }
};
*/

}

}
}

#endif
