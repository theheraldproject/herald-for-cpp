//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/engine/coordinator.h"

#include <memory>

namespace herald {
namespace engine {

bool operator<(const std::reference_wrapper<CoordinationProvider> first, const std::reference_wrapper<CoordinationProvider> second)
{
  return &(first.get()) < &(second.get()); // use reference_wrapper's call operator to fetch the underlying reference
}

}
}