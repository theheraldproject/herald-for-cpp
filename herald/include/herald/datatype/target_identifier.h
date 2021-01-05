//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef TARGET_IDENTIFIER_H
#define TARGET_IDENTIFIER_H

#include "data.h"

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class TargetIdentifier {
public:
  TargetIdentifier();
  TargetIdentifier(const Data& data);
  TargetIdentifier(const TargetIdentifier& from); // copy ctor
  ~TargetIdentifier();

  bool operator==(const TargetIdentifier& other) const noexcept;
  bool operator==(const Data& other) const noexcept;
  bool operator!=(const TargetIdentifier& other) const noexcept;
  bool operator!=(const Data& other) const noexcept;

  std::size_t hashCode() const;

  operator std::string() const;

  operator Data() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;

};

} // end namespace
} // end namespace



namespace std {
  template<>
  struct hash<herald::datatype::TargetIdentifier>
  {
    size_t operator()(const herald::datatype::TargetIdentifier& v) const
    {
      return v.hashCode();
    }
  };
} // end namespace

#endif