//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_PLATFORM_H
#define HERALD_PLATFORM_H

#include <exception>

/// Location for platform specific hacks / workarounds

#ifdef __ZEPHYR__

void Terminate() noexcept;

#endif

#endif