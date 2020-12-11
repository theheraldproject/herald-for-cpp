//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BASE64_STRING_H
#define BASE64_STRING_H

#include "data.h"

#include <string>

namespace herald {
namespace datatype {

/// Strongly, rather than stringly, typed representation of Base64 String data.
/// Prevents incorrect initialisation or manipulation
class Base64String {
public:
  Base64String(); // empty string initialiser
  Base64String(Base64String&& other);
  ~Base64String();

  static bool from(const std::string& original, Base64String& toInitialise) noexcept; // initialise from string
  static Base64String encode(const Data& from) noexcept; // initialise from Data

  Data decode() const noexcept;
  std::string encoded() const noexcept; // Return base64 string representation (copy of, not reference to)
private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};

} // end namespace
} // end namespace

#endif