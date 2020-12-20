//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//


#ifndef DIRECT_MIXING_H
#define DIRECT_MIXING_H

#include "../providers/social_mixing.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace heraldns {
namespace mixing {

using namespace heraldns::providers;

class DirectMixingScoreProvider : public SocialMixingScoreProvider {
public:
  DirectMixingScoreProvider(const PresenceManager& pm, std::shared_ptr<Grid> grid, double initialScore, double dropOffPerDay);
  ~DirectMixingScoreProvider() = default;

  void initialiseRiskScore(std::shared_ptr<Presence> presence);
  void calculateNewRiskScore(std::shared_ptr<Presence> presence, double minutesPassed);

private:
  const PresenceManager& m_pm;
  double m_initial;
  double m_dropoffPerMinute; // more efficient
  std::shared_ptr<Grid> m_grid;
};

} // end namespace
} // end namespace

#endif