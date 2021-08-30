//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

/*
 * The main executable of the herald-network-simulation process
 */
#include <iostream>
#include "../heraldns/heraldns.h"

int main(int argc, char* argv[]) {
  using namespace heraldns;
  using namespace heraldns::simulator;
  using namespace heraldns::datatype;
  using namespace heraldns::intermediate;
  using namespace heraldns::mixing;
  using namespace heraldns::transmission;

  PresenceManager pm(100);

  std::shared_ptr<Grid> grid = std::make_shared<Grid>(5, 5, 0.5);

  std::shared_ptr<DirectMixingScoreProvider> scoring = std::make_shared<DirectMixingScoreProvider>(
    pm, 
    grid, 
    100, // initial risk score
    1.0 / 14.0 // risk score drop off per day
  );
  std::shared_ptr<BasicTransmissionModelProvider> transmission = std::make_shared<BasicTransmissionModelProvider>(
    pm,
    grid, 
    14 * 24 * 60 / 5, // recovery: 14 days worth of ticks, with tick every 5 mins
    90 * 24 * 60 / 5, // immunity: 90 days worth of ticks, with tick every 5 mins
    25 // initial cases
  );

  std::shared_ptr<StdOutIntermediateResults> ir = std::make_shared<StdOutIntermediateResults>();
  
  Simulation sim(grid, pm, scoring, transmission);

  sim.runToCompletion(50, 60 * 5, ir, 14400/50); // 200 days, 1 day = 5 minutes per tick = 57600 ticks

  sim.writeStandardResults("./");

  return 0;
}
