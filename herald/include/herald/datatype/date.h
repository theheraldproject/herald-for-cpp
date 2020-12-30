//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DATE_H
#define DATE_H

#include <string>
#include <memory>

namespace herald {
namespace datatype {

// Basic date representation. Uses epoch seconds since Jan 1st 1970. Subclasses
// can use different mechanisms if they wish
class Date {
public:
  Date(); // now
  Date(long secondsSinceEpoch);
  Date(const Date& from);
  ~Date();

  std::string iso8601DateTime() const;
  std::string toString() const;
  long secondsSinceUnixEpoch() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif