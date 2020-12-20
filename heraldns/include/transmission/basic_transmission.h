//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//


#ifndef BASIC_TRANSMISSION_H
#define BASIC_TRANSMISSION_H

#include "../providers/transmission.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace heraldns {
namespace transmission {

using namespace heraldns::providers;

class BasicTransmissionModelProvider : public TransmissionModelProvider {
public:
  BasicTransmissionModelProvider(const PresenceManager& pm, std::shared_ptr<Grid> grid,
    uint64_t ticksToRecover, uint64_t ticksForImmunity, uint64_t initialInfections);
  ~BasicTransmissionModelProvider() = default;

  void initialiseInfectionState(std::shared_ptr<Presence> presence) override;
  void determineInfectionState(std::shared_ptr<Presence> presence, double minutesPassed, uint64_t tick) override;

private:
  const PresenceManager& m_pm;
  std::shared_ptr<Grid> m_grid;
  uint64_t m_ticksToRecover;
  uint64_t m_ticksForImmunity;
  uint64_t m_initialInfections;
  uint64_t m_assignedInfections;
};

} // end namespace
} // end namespace

#endif