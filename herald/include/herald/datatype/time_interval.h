//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef TIME_INTERVAL_H
#define TIME_INTERVAL_H

#include "date.h"

#include <string>
#include <memory>

#include <cstdint>
#include <climits>

namespace herald {
namespace datatype {

class TimeInterval {
public:
  static TimeInterval minutes(long minutes);
  static TimeInterval seconds(long seconds);
  static TimeInterval never();

  TimeInterval(long seconds);
  TimeInterval(const Date& date);
  TimeInterval(const Date& from, const Date& to);

  ~TimeInterval();

  TimeInterval operator*(const TimeInterval& other) noexcept;
  TimeInterval operator+(const TimeInterval& other) noexcept;
  TimeInterval operator-(const TimeInterval& other) noexcept;
  
  bool operator>(const TimeInterval& other) const noexcept;
  bool operator>=(const TimeInterval& other) const noexcept;
  bool operator<(const TimeInterval& other) const noexcept;
  bool operator<=(const TimeInterval& other) const noexcept;
  bool operator==(const TimeInterval& other) const noexcept;
  bool operator!=(const TimeInterval& other) const noexcept;

  long millis() const noexcept;

  operator std::string() const noexcept;
  operator long() const noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif