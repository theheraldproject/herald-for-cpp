//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef INTERMEDIATE_RESULTS_H
#define INTERMEDIATE_RESULTS_H

#include "../datatypes/presence.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace heraldns {
namespace providers {

using namespace heraldns::datatype;

class IntermediateResultsListener {
public:
  IntermediateResultsListener() = default;
  virtual ~IntermediateResultsListener() = default;

  virtual void intermediateResults(uint64_t casesNow, uint64_t recoveredNow, 
    const PresenceManager& pm,
    double minutesPassed, uint64_t ticksComplete) = 0;
};

}
}

#endif