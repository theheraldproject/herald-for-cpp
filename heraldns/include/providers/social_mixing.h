//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SOCIAL_MIXING_H
#define SOCIAL_MIXING_H

#include "../datatypes/presence.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace heraldns {
namespace providers {

using namespace heraldns::datatype;

class SocialMixingScoreProvider {
public:
  SocialMixingScoreProvider() = default;
  virtual ~SocialMixingScoreProvider() = default;

  virtual void initialiseRiskScore(std::shared_ptr<Presence> presence) = 0;
  virtual void calculateNewRiskScore(std::shared_ptr<Presence> presence, double minutesPassed) = 0;
};

}
}

#endif