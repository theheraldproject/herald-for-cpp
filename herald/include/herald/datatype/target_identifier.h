//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TARGET_IDENTIFIER_H
#define HERALD_TARGET_IDENTIFIER_H

#include "data.h"

#include <string>
#include <memory>
#include <iosfwd>

namespace herald {
namespace datatype {

class TargetIdentifier {
public:
  TargetIdentifier();
  TargetIdentifier(const Data& data);
  TargetIdentifier(const TargetIdentifier& from); // copy ctor
  TargetIdentifier& operator=(const TargetIdentifier& from); // copy assign
  ~TargetIdentifier();

  bool operator==(const TargetIdentifier& other) const noexcept;
  bool operator==(const Data& other) const noexcept;
  bool operator!=(const TargetIdentifier& other) const noexcept;
  bool operator!=(const Data& other) const noexcept;
  bool operator<(const TargetIdentifier& other) const noexcept; // required for std::less
  bool operator>(const TargetIdentifier& other) const noexcept; // required for std::less

  std::size_t hashCode() const;

  operator std::string() const;

  operator Data() const;

private:
  Data value;

};

} // end namespace
} // end namespace



namespace std {
  inline std::ostream& operator<<(std::ostream &os, const herald::datatype::TargetIdentifier& d)
  {
    return os << ((herald::datatype::Data)d).reversed().hexEncodedString();
  }

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