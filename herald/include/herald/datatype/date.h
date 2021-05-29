//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DATE_H
#define HERALD_DATE_H

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class TimeInterval; // fwd decl

// Basic date representation. Uses epoch seconds since Jan 1st 1970. Subclasses
// can use different mechanisms if they wish
class Date {
public:
  Date(); // now
  Date(std::uint64_t secondsSinceEpochOrRestart);
  Date(const Date& from);
  Date(Date&& from);
  ~Date();

  Date& operator=(const Date& other) noexcept;
  Date& operator=(Date&& other) noexcept;

  std::string iso8601DateTime() const noexcept;
  operator std::string() const noexcept;
  std::uint64_t secondsSinceUnixEpoch() const noexcept;

  bool operator==(const Date& other) const noexcept;
  bool operator!=(const Date& other) const noexcept;
  bool operator<(const Date& other) const noexcept;
  bool operator>(const Date& other) const noexcept;
  bool operator<=(const Date& other) const noexcept;
  bool operator>=(const Date& other) const noexcept;

  Date operator-(const TimeInterval& other) noexcept;
  Date& operator-=(const TimeInterval& other) noexcept;
  Date operator+(const TimeInterval& other) noexcept;
  Date& operator+=(const TimeInterval& other) noexcept;

  operator long() const noexcept;

private:
  std::uint64_t seconds;
};

} // end namespace
} // end namespace

#endif