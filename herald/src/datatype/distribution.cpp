//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/distribution.h"

#include <limits>
#include <cmath>

namespace herald {
namespace datatype {

Distribution::Distribution() noexcept
  : n(0),
    m1(0.0),
    m2(0.0),
    minimum(std::numeric_limits<double>::max()),
    maximum(std::numeric_limits<double>::min())
{
  ;
}

Distribution::Distribution(double x, std::size_t frequency) noexcept
  : n(frequency),
    m1(x),
    m2(0),
    minimum(x),
    maximum(x)
{
  ;
}

void
Distribution::add(double x) noexcept
{
  // Update count, mean, variance
  std::size_t n1 = n;
  ++n;
  double delta = x - m1;
  double delta_n = delta / n;
  double term1 = delta * delta_n * n1;
  m1 += delta_n;
  m2 += term1;
  // update min, max
  if (x < minimum) {
    minimum = x;
  }
  if (x > maximum) {
    maximum = x;
  }
}

void
Distribution::add(double x, std::size_t frequency) noexcept
{
  for (std::size_t i = 0;i < frequency;++i) {
    add(x);
  }
}

void
Distribution::add(const Distribution& other) noexcept
{
  if (0 == n) {
    n = other.n;
    m1 = other.m1;
    m2 = other.m2;
    minimum = other.minimum;
    maximum = other.maximum;
    return;
  }
  // Combine distribution if this distribution is not empty
  std::size_t new_n = n + other.n;

  double delta = other.m1 - m1;
  double delta2 = delta * delta;

  m1 = ((n * m1) + (other.n * other.m1)) / new_n;
  m2 = m2 + other.m2 + ((delta2 * n * other.n) / new_n);
  
  // set values
  n = new_n;
  minimum = minimum < other.minimum ? minimum : other.minimum;
  maximum = maximum > other.maximum ? maximum : other.maximum;
}

const std::size_t
Distribution::count() const noexcept
{
  return n;
}

const double
Distribution::mean() const noexcept
{
  return m1;
}

const double
Distribution::variance() const noexcept
{
  if (n < 2) { // guard
    return 0;
  }
  return m2 / (n - 1);
}

const double
Distribution::standardDeviation() const noexcept
{
  if (n < 2) { // guard
    return 0;
  }
  return std::sqrt(variance());
}

const double
Distribution::min() const noexcept
{
  return minimum;
}

const double
Distribution::max() const noexcept
{
  return maximum;
}

Distribution::operator std::string() const noexcept
{
  return "[count=" + std::to_string(count()) + ",mean=" + std::to_string(mean()) + 
         ",sd=" + std::to_string(standardDeviation()) + ",min=" + std::to_string(min()) +
         ",max=" + std::to_string(max()) + "]";
}

}
}
