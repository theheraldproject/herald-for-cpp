//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ZEPHYR_CONTEXT_H
#define ZEPHYR_CONTEXT_H

// #ifdef __ZEPHYR__

#include "context.h"

#include <memory>
#include <iosfwd>
#include <string>

namespace herald {

/*
 * Zephyr context class - holds state generic across our application for a particular device.
 */
class ZephyrContext : public Context {
public:
  ZephyrContext();
  ~ZephyrContext();

  std::ostream& getLoggingSink(const std::string& requestedFor) override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL idiom
};




} // end namespace

// #endif

#endif