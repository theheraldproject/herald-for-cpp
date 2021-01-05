//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <string>

namespace herald {
namespace datatype {

// Concept of an error code
class ErrorCode {
public:
  ErrorCode() : mSuccess(true), mMessage("") { }
  ErrorCode(bool success) : mSuccess(success), mMessage("") { }
  ErrorCode(bool success, std::string message) : mSuccess(success), mMessage(message) { }
  ~ErrorCode() = default;

  bool operator()() {
    return mSuccess;
  }

  std::string message() {
    return mMessage;
  }

private:
  bool mSuccess;
  std::string mMessage;
};

} // end namespace
} // end namespace

#endif