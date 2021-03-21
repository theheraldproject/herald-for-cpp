//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/base64_string.h"
#include "herald/datatype/data.h"

#include <string>
#include <iosfwd>

namespace herald {
namespace datatype {

// IMPL DEFINITION
// class Base64String::Impl {
// public:
//   Impl();
//   ~Impl() = default;

//   std::string value; // Base64 encoded, and guarded
// };

// Base64String::Impl::Impl() { }


const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


bool is_base64(char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}




bool
Base64String::from(const std::string& original, Base64String& toInitialise) noexcept {
  bool ok = true;
  for (auto& c : original) {
    ok = ok & is_base64(c);
  }
  if (!ok) {
    return false;
  }
  toInitialise.value = original;
  return true;
}

Base64String::Base64String() : value() { }

Base64String::Base64String(Base64String&& other)
 : value(std::move(other.value))
{
  ;
}

Base64String::~Base64String() = default;

Base64String 
Base64String::encode(const Data& from) noexcept {
  std::size_t bufLen = from.size();
  std::string ret;
  ret.reserve(from.size());
  int i = 0;
  int j = 0;
  char char_array_3[3];
  char char_array_4[4];

  std::size_t idx = 0;
  while (bufLen--) {
    char_array_3[i++] = (char)from.at(idx);
    idx++;
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++) {
        ret += base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) {
      ret += base64_chars[char_array_4[j]];
    }

    while((i++ < 3)) {
      ret += '=';
    }
  }

  // return ret;

  // std::string buffer; // should be smaller, but this will do
  // buffer.reserve(length);
  // std::size_t pad = 0;
  // for (std::size_t i = 0; i < length; i += 3) {

  //   int b = ((((int)data[i]) & 0xFF) << 16) & 0xFFFFFF;
  //   if (i + 1 < length) {
  //     b |= (((int)data[i + 1]) & 0xFF) << 8;
  //   } else {
  //     pad++;
  //   }
  //   if (i + 2 < length) {
  //     b |= (((int)data[i + 2]) & 0xFF);
  //   } else {
  //     pad++;
  //   }

  //   for (int j = 0; j < 4 - pad; j++) {
  //     std::size_t c = (b & 0xFC0000) >> 18;
  //     buffer.append(tbl[c]);
  //     b <<= 6;
  //   }
  // }
  // for (std::size_t j = 0; j < pad; j++) {
  //   buffer.append("=");
  // }

  Base64String nvalue;
  nvalue.value = std::move(ret);
  return nvalue;
}

Data
Base64String::decode() const noexcept {
  std::size_t in_len = value.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  char char_array_4[4], char_array_3[3];
  std::vector<std::byte> ret;

  while (in_len-- && ( value[in_] != '=') && is_base64(value[in_])) {
    char_array_4[i++] = value[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (char)base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) {
        ret.push_back(std::byte(char_array_3[i]));
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = (char)base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(std::byte(char_array_3[j]));
  }

  return Data(ret); // std::move via compiler
}


std::string
Base64String::encoded() const noexcept {
  return value; // copy ctor
}


} // end namespace
} // end namespace
