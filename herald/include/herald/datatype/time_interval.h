//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TIME_INTERVAL_H
#define HERALD_TIME_INTERVAL_H

#include "date.h"

#include <string>
#include <memory>

#include <cstdint>
#include <climits>

namespace herald {
namespace datatype {

class TimeInterval {
public:
  static TimeInterval hours(long hours);
  static TimeInterval minutes(long minutes);
  static TimeInterval seconds(long seconds);
  static TimeInterval never();
  static TimeInterval zero();

  TimeInterval(long seconds);
  TimeInterval(const Date& date);
  TimeInterval(const Date& from, const Date& to);
  TimeInterval(const TimeInterval& other); // copy ctor

  ~TimeInterval();

  TimeInterval& operator=(const TimeInterval& other) noexcept; // copy assign

  TimeInterval operator*(const TimeInterval& other) noexcept;
  TimeInterval operator+(const TimeInterval& other) noexcept;
  TimeInterval& operator+=(const TimeInterval& other) noexcept;
  TimeInterval operator-(const TimeInterval& other) noexcept;
  TimeInterval& operator-=(const TimeInterval& other) noexcept;
  TimeInterval operator*(double multiplier) noexcept;
  TimeInterval& operator*=(double multiplier) noexcept;
  /** If divisor is zero, makes no modification. **/
  TimeInterval operator/(double divisor) noexcept;
  /** If divisor is zero, makes no modification. **/
  TimeInterval& operator/=(double divisor) noexcept;
  
  bool operator>(const TimeInterval& other) const noexcept;
  bool operator>=(const TimeInterval& other) const noexcept;
  bool operator<(const TimeInterval& other) const noexcept;
  bool operator<=(const TimeInterval& other) const noexcept;
  bool operator==(const TimeInterval& other) const noexcept;
  bool operator!=(const TimeInterval& other) const noexcept;

  long millis() const noexcept;
  long seconds() const noexcept;

  operator std::string() const noexcept;
  operator long() const noexcept; // returns SECONDS not millis

private:
  long secs;
};

} // end namespace
} // end namespace

#endif