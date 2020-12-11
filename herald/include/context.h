//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <iosfwd>
#include <string>

namespace herald {

// TODO figure out what this means for our various implementations
class Context {
public:
  Context() = default;
  virtual ~Context() = default;

  virtual std::ostream& getLoggingSink(const std::string& requestedFor) = 0;
};




} // end namespace

#endif