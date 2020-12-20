//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include "../../heraldns.h"

using namespace heraldns;
using namespace heraldns::datatype;
using namespace heraldns::transmission;

namespace heraldns {
namespace transmission {


BasicTransmissionModelProvider::BasicTransmissionModelProvider(const PresenceManager& pm, std::shared_ptr<Grid> grid,
  uint64_t ticksToRecover, uint64_t ticksForImmunity, uint64_t initialInfections)
  : m_pm(pm),
    m_grid(grid),
    m_ticksToRecover(ticksToRecover),
    m_ticksForImmunity(ticksForImmunity),
    m_initialInfections(initialInfections),
    m_assignedInfections(0)
{
  ;
}

void
BasicTransmissionModelProvider::initialiseInfectionState(std::shared_ptr<Presence> presence)
{
  if (m_assignedInfections < m_initialInfections) {
    presence->newState(State::Ill, 0);
    m_assignedInfections++;
    std::cout << "I";
  } else {
    presence->newState(State::Well, 0);
  }
}

void
BasicTransmissionModelProvider::determineInfectionState(
  std::shared_ptr<Presence> presence, double minutesPassed, uint64_t tick)
{
  // first check to see if we've been recovered for long enough to fall ill again (90 days for now)
  State currentState = presence->state();
  // first, check if we still have immunity (short lived)
  if (currentState == State::Recovered && presence->lastFellIll() + m_ticksForImmunity >= tick) {
    currentState = State::Well;
  } else if (currentState == State::Ill && presence->lastFellIll() + m_ticksToRecover >= tick) {
    // now, check if we're still ill and have recovered (we don't do deaths yet)
    currentState = State::Recovered;
  }
  
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

  double oxfordRiskScore = presence->transmissionModelScore();

  for (uint64_t cx = minX;cx <= maxX;cx++) {
    for (uint64_t cy = minY;cy <= maxY;cy++) {
      // TODO Check cell is definitely in range (we loop over a box, radius is a circle)
      
      // If so, for each presence in the cell
      auto cell = m_grid->cell(cx,cy);
      for (auto pOtherId : cell->present()) {
        std::shared_ptr<Presence> pOther = m_pm.get(pOtherId);
        if (pOtherId != presence->id() && pOther->state() == State::Ill) {
          distance = m_grid->distance(cell, presence->position());
          oxfordRiskScore += minutesPassed * 
            4.0 * // See risk-model-approximations for 4.0 coefficient explanation
            (distance <= 1.0 ? 1.0 : 1.0 / pow(distance, 2.0))
          ; // TODO change to actual oxford risk model formula
        }
      }
    }
  }
  presence->newTransmissionModelScore(oxfordRiskScore);
  // Has this person *actually* fallen ill?
  if (currentState == State::Well && oxfordRiskScore > 60) { // number for above if 15m @ 2m (4 * inv dist sq)
    presence->newState(State::Ill, tick);
  } else {
    presence->newState(currentState, tick);
  }
}


}
}
