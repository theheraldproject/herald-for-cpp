//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/context.h"

#include <iostream>

namespace herald {

std::ostream&
DefaultContext::getLoggingSink(const std::string& requestedFor)
{
  return std::cout;
}

}