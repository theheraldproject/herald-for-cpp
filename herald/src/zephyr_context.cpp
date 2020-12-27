//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "zephyr_context.h"

#include <memory>
#include <iostream>

// #ifdef __ZEPHYR__

namespace herald {

class ZephyrContext::Impl {
public:
  Impl();
  ~Impl();

  // TODO Any Zephyr RTOS specific global handles go here
};

ZephyrContext::Impl::Impl()
{
  ;
}

ZephyrContext::Impl::~Impl()
{
  ;
}


// Zephyr RTOS implementation of Context
ZephyrContext::ZephyrContext()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

ZephyrContext::~ZephyrContext()
{
  ;
}

std::ostream& 
ZephyrContext::getLoggingSink(const std::string& requestedFor)
{
  // TODO return a better stream for this platform (E.g. Serial output)
  return std::cout;
}

} // end namespace

// #endif
