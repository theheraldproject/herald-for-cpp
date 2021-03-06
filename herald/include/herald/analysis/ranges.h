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

namespace herald {
namespace analysis {

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
          typename IterT = typename Coll::iterator
         >
struct iterator_proxy {
  iterator_proxy(Coll& coll) : iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))) {}
  iterator_proxy(iterator_proxy&& other) : iter(std::move(other.iter)), endIter(std::move(other.endIter)) {}
  iterator_proxy(const iterator_proxy& other) : iter(other.iter), endIter(other.endIter) {}
  ~iterator_proxy() = default;

  auto operator*() -> ValT& {
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

private:
  const Pred pred;
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
          typename IterT = typename Coll::iterator
         >
struct filtered_iterator_proxy {
  // const value_type = typename ValT;
  // const iterator = typename IterT;

  filtered_iterator_proxy(Coll& coll, Pred pred) : iter(std::move(std::begin(coll))), endIter(std::move(std::end(coll))), filter(pred) {
    // move forward to the first match (or end)
    moveToNext();
  }
  filtered_iterator_proxy(filtered_iterator_proxy&& other) : iter(std::move(other.iter)), endIter(std::move(other.endIter)), filter(other.filter) {}
  filtered_iterator_proxy(const filtered_iterator_proxy& other) : iter(other.iter), endIter(other.endIter), filter(other.filter) {}
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


private:
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

/// Create a view holder that wraps an iterator, so the result of all returns have a begin() and end()
/// just like an STL collection
template <typename IterProxyT>
struct view {
  view(IterProxyT&& srcIter) : source(std::move(srcIter)) {} // TAKE OWNERSHIP
  ~view() = default;

  auto begin() -> IterProxyT {
    return source;
  }

  auto end() -> IterProxyT {
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

private:
  IterProxyT source;
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

  template <typename Coll>
  friend auto operator|(Coll c,filter<Pred> pred) -> filtered_iterator_proxy<Coll,Pred> {
    return filtered_iterator_proxy<Coll,Pred>(c,pred.pred);
  }

  // template <typename Coll>
  // friend auto operator|(Coll c,filter<Pred> pred) -> view<filtered_iterator_proxy<Coll,Pred>> {
  //   return view<filtered_iterator_proxy<Coll,Pred>>(filtered_iterator_proxy<Coll,Pred>(c,pred.pred));
  // }

private:
  Pred pred;
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
