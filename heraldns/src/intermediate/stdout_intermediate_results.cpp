//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "../../heraldns.h"

#include <iostream>

namespace heraldns {
namespace intermediate {

using namespace heraldns;
using namespace heraldns::datatype;

void
StdOutIntermediateResults::intermediateResults(uint64_t casesNow, uint64_t recoveredNow, 
  const PresenceManager& pm,
  double minutesPassed, uint64_t ticksComplete)
{
  std::cout << "Tick: " << ticksComplete << ", Current Cases: " << casesNow << ", Recovered: " << recoveredNow << std::endl;
}

}
}
