//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef LOCATION_H
#define LOCATION_H

#include "location_reference.h"
#include "date.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

template <typename LocationReferenceT>
class Location {
public:
  Location(LocationReferenceT&& value, Date&& start, Date&& end)
    : mValue(std::move(value)),mStart(std::move(start)),mEnd(std::move(end))
  {};
  ~Location();

  std::string description() const {
    return mValue->description() + ":[from=" + ((std::string)mStart) + ",to=" + ((std::string)mEnd) + "]";
  }
  
  operator std::string() const noexcept {
    return description();
  }

private:
  // class Impl;
  // std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
  LocationReferenceT mValue;
  Date mStart;
  Date mEnd;
};

} // end namespace
} // end namespace

#endif