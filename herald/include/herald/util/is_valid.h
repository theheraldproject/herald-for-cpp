//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_IS_VALID_H
#define HERALD_IS_VALID_H

#include <utility>

namespace herald {
namespace util {

template<typename F, typename... Args, typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

template<typename F, typename... Args>
std::false_type isValidImpl(...);

/// \brief Template check for existance of a function
/// \since v2.1.0
inline constexpr auto isValid = [] (auto f) {
  return [](auto&&... args) {
    return decltype(isValidImpl<decltype(f),decltype(args)&&...>(nullptr)){};
  };
};

template<typename T>
struct TypeT {
  using Type = T;
};

template<typename T>
constexpr auto type = TypeT<T>{};

template<typename T>
T valueT(TypeT<T>);



/// MARK: Comparison standard function checks

constexpr auto hasEqualityFunction = isValid(
  [](auto&& s,auto&& lhs,auto&& rhs) ->
    decltype(((decltype(s))s).get().operator==(lhs,rhs)) {}
);
template<typename T,typename... Args>
using HasEqualityFunctionT = decltype(hasEqualityFunction(std::declval<T>(),std::declval<Args>()...));
template<typename T,typename... Args>
constexpr auto HasEqualityFunctionV = HasEqualityFunctionT<T,Args...>::value;


constexpr auto hasInequalityFunction = isValid(
  [](auto&& s,auto&& lhs,auto&& rhs) ->
    decltype(((decltype(s))s).get().operator!=(lhs,rhs)) {}
);
template<typename T,typename... Args>
using HasInequalityFunctionT = decltype(hasInequalityFunction(std::declval<T>(),std::declval<Args>()...));
template<typename T,typename... Args>
constexpr auto HasInequalityFunctionV = HasInequalityFunctionT<T,Args...>::value;



}
}

#endif