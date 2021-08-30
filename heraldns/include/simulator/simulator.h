//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "../datatypes/presence.h"
#include "../datatypes/grid.h"
#include "../providers/social_mixing.h"
#include "../providers/transmission.h"
#include "../providers/intermediate_results.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <random>

namespace heraldns {
namespace simulator {

using namespace heraldns::datatype;
using namespace heraldns::providers;

class Simulation {
public:
  Simulation(std::shared_ptr<Grid> grid, const PresenceManager& pm,
             std::shared_ptr<SocialMixingScoreProvider> scoring,
             std::shared_ptr<TransmissionModelProvider> transmission);
  Simulation(const Simulation& from); // copy ctor
  ~Simulation() = default;

  void runToCompletion(uint64_t days, uint64_t secondsPerTick);
  void runToCompletion(uint64_t days, uint64_t secondsPerTick, 
    std::shared_ptr<IntermediateResultsListener> callback, uint64_t ticksPerCallback); // for future UI

  bool writeStandardResults(std::string outputFolder) noexcept; // returns success = true

private:
  // methods
  void reset(uint64_t days, uint64_t secondsPerTick); // resets the sim before beginning
  void tick(); // perform a single tick in the simulation

  // initial settings
  std::shared_ptr<Grid> m_grid;

  // providers
  std::shared_ptr<SocialMixingScoreProvider> scoreProvider;
  std::shared_ptr<TransmissionModelProvider> modelProvider;

  // settings
  uint64_t maxTicks;
  double minutesPerTick;

  // Runtime variables
  uint64_t currentTick;
  uint64_t today; // day number. 0 = start

  const PresenceManager& m_pm;

  // results variables/aggregations that sit outside of an individual Presence
  std::vector<uint64_t> casesPerDay; // day 0 = initial values, day 1 = end of first day of simulation
  std::vector<uint64_t> recoveredPerDay;
  
  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<int64_t> distrib;
};

} // end namespace
} // end namespace

#endif