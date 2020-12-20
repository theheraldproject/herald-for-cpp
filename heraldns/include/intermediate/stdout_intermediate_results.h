//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef STDOUT_INTERMEDIATE_RESULTS_H
#define STDOUT_INTERMEDIATE_RESULTS_H

#include "../providers/intermediate_results.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace heraldns {
namespace intermediate {

using namespace heraldns::providers;

class StdOutIntermediateResults : public IntermediateResultsListener {
public:
  StdOutIntermediateResults() = default;
  ~StdOutIntermediateResults() = default;

  void intermediateResults(uint64_t casesNow, uint64_t recoveredNow, 
    const PresenceManager& pm,
    double minutesPassed, uint64_t ticksComplete) override;

private:
};

} // end namespace
} // end namespace

#endif