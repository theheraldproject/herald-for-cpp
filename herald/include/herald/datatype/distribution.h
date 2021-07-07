//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DISTRIBUTION_H
#define HERALD_DISTRIBUTION_H

#include <cstddef>
#include <string>

namespace herald {
namespace datatype {

/// /brief Simple distance value in metres
/// Has to be a class/struct type to allow template resolution
class Distribution {
public:
  /// Initialise an empty distribution
  Distribution() noexcept;
  /// Initialise a distribution with frequency occurences of value x
  Distribution(double x, std::size_t frequency) noexcept;
  /// Destroy a distribution
  ~Distribution() noexcept = default;

  /// Add a single occurence of a value
  void add(double x) noexcept;
  /// Add multiple occurences of a value
  void add(double x, std::size_t frequency) noexcept;
  /// Add another distribution to this value
  void add(const Distribution& other) noexcept;

  /// return the count
  const std::size_t count() const noexcept;
  /// return the mean
  const double mean() const noexcept;
  /// return the variance
  const double variance() const noexcept;
  /// Return the standard deviation
  const double standardDeviation() const noexcept;
  /// Return the minimum recorded value
  const double min() const noexcept;
  /// Return the maximum recorded value
  const double max() const noexcept;

  /// Convert this distribution to a string
  operator std::string() const noexcept;

private:
  /// Accumulator for count
  std::size_t n;
  /// Accumulator for mean
  double m1;
  /// Accumulator for variance
  double m2;
  /// Minimum recorded value
  double minimum;
  /// Maximum recorded value
  double maximum;
};

}
}

#endif
