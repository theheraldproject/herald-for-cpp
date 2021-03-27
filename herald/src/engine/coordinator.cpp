//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/engine/coordinator.h"

#include <memory>

namespace herald {
namespace engine {

bool operator<(const std::shared_ptr<CoordinationProvider>& first, const std::shared_ptr<CoordinationProvider>& second)
{
  return &(*first) < &(*second); // simple memory address comparator of item pointed TO
}

}
}