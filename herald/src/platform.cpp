//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/platform.h"

#ifdef __ZEPHYR__

void Terminate() noexcept
{
  // Does nothing
}

namespace __cxxabiv1
{
    std::terminate_handler __terminate_handler = Terminate;
}

#endif