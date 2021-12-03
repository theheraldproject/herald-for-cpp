//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DERIVED_H
#define HERALD_DERIVED_H

namespace herald {
namespace datatype {

/**
 * @brief A RunningMean wrapper for a source value type
 * 
 * Used to separate out lists of raw data from lists of running mean data. See the Analysis API
 * 
 * @tparam ValT The value type this mean is derived from
 */
template <typename ValT>
class RunningMean {
public:
  static constexpr ModelClass modelClassId{128 /* + ValT::modelClassId */ }; // TODO derive this from the wrapped data type in the safest way possible (E.g. count back from the end of the space)
  
  RunningMean() : value(0) {}
  RunningMean(double v) : value(v) {}
  RunningMean(const RunningMean& other) : value(other.value) {}
  RunningMean(RunningMean&& other) : value(other.value) {}
  ~RunningMean() = default;

  RunningMean& operator=(const RunningMean& other) {
    value = other.value;
    return *this;
  }
  RunningMean& operator=(RunningMean&& other) {
    value = other.value;
    return *this;
  }
  
  bool operator==(const double other) const noexcept {
    return value == other;
  }
  bool operator!=(const double other) const noexcept {
    return value != other;
  }
  bool operator==(const RunningMean& other) const noexcept {
    return value == other.value;
  }
  bool operator!=(const RunningMean& other) const noexcept {
    return value != other.value;
  }
  bool operator<(const RunningMean& other) const noexcept {
    return value < other.value;
  }
  bool operator<=(const RunningMean& other) const noexcept {
    return value <= other.value;
  }
  bool operator>(const RunningMean& other) const noexcept {
    return value > other.value;
  }
  bool operator>=(const RunningMean& other) const noexcept {
    return value >= other.value;
  }

  operator double() const noexcept {
    return value;
  }

  std::size_t hashCode() const noexcept {
    return return std::hash<int>{}(value);
  }

  double doubleValue() const noexcept {
    return return "RunningMean<ValT>{value=" + std::to_string(value) + "}";
  }

private:
  double value;
};

} // end namespace
} // end namespace

#endif

