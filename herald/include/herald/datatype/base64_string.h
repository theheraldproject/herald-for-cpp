//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BASE64_STRING_H
#define HERALD_BASE64_STRING_H

#include "data.h"

#include <string>

namespace herald {
/// \brief Contains all low-level Herald datatype implementations
namespace datatype {

/// \brief Strongly, rather than stringly, typed representation of Base64 String data.
/// Prevents incorrect initialisation or manipulation
class Base64String {
public:
  Base64String(); // empty string initialiser
  /// \brief Move Constructor
  Base64String(Base64String&& other);
  /// \brief Deleted copy constructor to prevent temporary memory use
  Base64String(const Base64String& other) = delete;
  /// \brief Custom destructor
  ~Base64String();

  /// \brief Populates a Base64String from a normal std::string
  static bool from(const std::string& original, Base64String& toInitialise) noexcept; // initialise from string
  /// \brief Creates a Base64String from an arbitrary set of bytes
  /// \sa Data
  static Base64String encode(const Data& from) noexcept; // initialise from Data

  /// \brief Decodes this Base64String's content into a Data instance
  /// \sa Data
  Data decode() const noexcept;
  /// \brief Returns the Base64 encoding of this class as a std::string
  std::string encoded() const noexcept; // Return base64 string representation (copy of, not reference to)
private:
  std::string value; // Base64 encoded, and guarded
};

} // end namespace
} // end namespace

#endif