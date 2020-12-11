//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "datatype/data.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace herald {
namespace datatype {

// DATA PIMPL DEFINITIONS

class Data::Impl {
public:
  Impl();
  ~Impl() = default;

  std::vector<std::byte> data;
};

// DATA PIMPL DECLARATIONS
Data::Impl::Impl() : data() { }





// DATA CLASS DECLARATIONS
Data::Data() : mImpl(std::make_unique<Impl>()) {
  ;
}

Data::Data(const std::byte* value, std::size_t length) : mImpl(std::make_unique<Impl>()) {
  mImpl->data.reserve(length);
  for (std::size_t i = 0;i < length; i++) {
    mImpl->data[i] = value[i];
  }
}

Data::Data(std::vector<std::byte> value) : mImpl(std::make_unique<Impl>()) {
  mImpl->data = std::move(value);
}

Data::Data(const Data& from) : mImpl(std::make_unique<Impl>()) {
  mImpl->data = from.mImpl->data; // copy ctor
}

Data::Data(std::byte repeating, std::size_t count) : mImpl(std::make_unique<Impl>()) {
  mImpl->data.reserve(count);
  for (std::size_t i = 0;i < count; i++) {
    mImpl->data.push_back(repeating);
  }
}

Data::~Data() {}




std::string
Data::description() const {
  return ""; // TODO make this a real description
}

Data
Data::subdata(std::size_t offset) const {
  Data copy;
  if (offset > mImpl->data.size()) {
    return std::move(copy);
  }
  std::copy(mImpl->data.begin() + offset, mImpl->data.end(), std::back_inserter(copy.mImpl->data));
  return std::move(copy);
}

Data
Data::subdata(std::size_t offset, std::size_t length) const {
  Data copy;
  if (offset > mImpl->data.size()) {
    return std::move(copy);
  }
  std::copy(mImpl->data.begin() + offset, mImpl->data.begin() + length, std::back_inserter(copy.mImpl->data));
  return std::move(copy);
}

std::byte
Data::at(std::size_t index) const {
  if (mImpl->data.size() < (index - 1)) {
    return std::byte(0);
  }
  return mImpl->data[index];
}

void
Data::append(const Data& data) {
  mImpl->data.reserve(mImpl->data.size() + data.size());
  std::copy(data.mImpl->data.begin(), data.mImpl->data.end(), std::back_inserter(mImpl->data));
}

bool
Data::operator==(const Data& other) const noexcept {
  //if (hashCode() != other.hashCode()) {
  //  return false;
  //}
  // else compare each value

  // alternatively, cheat...
  return hashCode() == other.hashCode(); // Somewhat naughty
}


std::size_t
Data::hashCode() const {
  return std::hash<std::vector<std::byte>>{}(mImpl->data);
}

std::size_t
Data::size() const {
  return mImpl->data.size();
}

} // end namespace
} // end namespace