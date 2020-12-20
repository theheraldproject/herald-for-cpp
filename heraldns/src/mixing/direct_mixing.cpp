//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "../../heraldns.h"

#include <unordered_map>
#include <iostream>
#include <random>
#include <cmath>

using namespace heraldns;

namespace heraldns {
namespace mixing {

DirectMixingScoreProvider::DirectMixingScoreProvider(const PresenceManager& pm, std::shared_ptr<Grid> grid, double initialScore, double dropOffPerDay)
  : m_pm(pm), m_grid(grid), m_initial(initialScore), m_dropoffPerMinute(dropOffPerDay)
{
  ;
}

void
DirectMixingScoreProvider::initialiseRiskScore(std::shared_ptr<Presence> presence)
{
  presence->newRisk(m_initial);
}

void
DirectMixingScoreProvider::calculateNewRiskScore(std::shared_ptr<Presence> presence, 
  double minutesPassed)
{
  // calculate new risk score
  // Get Cell we're in
  std::shared_ptr<Cell> cell = presence->position();
  // Determine max distance for nearby cells (within infection risk range)
  uint64_t x = cell->x();
  uint64_t y = cell->y();
  // calculate out to 8 metres
  uint64_t radius = (uint64_t)std::ceil(8.0 / m_grid->separation());
  uint64_t minX = x - radius;
  if (minX < 0) minX = 0;
  uint64_t minY = y - radius;
  if (minY < 0) minY = 0;
  uint64_t maxX = x + radius;
  if (maxX > m_grid->width() - 1) maxX = m_grid->width() - 1;
  uint64_t maxY = y + radius;
  if (maxY > m_grid->height() - 1) maxY = m_grid->height() - 1;
  // For each cell
  double distance;
  std::unordered_map<uint64_t, double> observedTransmittedRisk;
  for (uint64_t cx = minX;cx <= maxX;cx++) {
    for (uint64_t cy = minY;cy <= maxY;cy++) {
      // TODO Check cell is definitely in range (we loop over a box, radius is a circle)
      
      // If so, for each presence in the cell
      auto cell = m_grid->cell(cx,cy);
      for (auto pOtherId : cell->present()) {
        if (pOtherId != presence->id()) {
          distance = m_grid->distance(cell, presence->position());
          std::shared_ptr<Presence> pOther = m_pm.get(pOtherId);
          observedTransmittedRisk.emplace(pOtherId, pOther->transmittedRisk());
          presence->newRisk(presence->newRisk() + 
            pOther->transmittedRisk() / 
            pow(
              (1.0 > distance ? 1 : distance), // Under 1 m all risk incurred is the same
            2.0) // inverse square for now TODO make this a similar scaling to Oxford model
          );
        }
      }
    }
  }

  double transmittedSum = 0.0;
  if (observedTransmittedRisk.size() > 0) {
    for (auto tPair : observedTransmittedRisk) {
      transmittedSum += tPair.second;
    }
    transmittedSum /= observedTransmittedRisk.size();
  }
  // now set our transmission value for the next tick
  presence->newTransmittedRisk(transmittedSum);
}


}
}
