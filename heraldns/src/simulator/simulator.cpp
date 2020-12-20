//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include "../../heraldns.h"

#include <cmath>

using namespace heraldns;
using namespace heraldns::simulator;
using namespace heraldns::datatype;
using namespace heraldns::mixing;
using namespace heraldns::transmission;

namespace heraldns {
namespace simulator {

Simulation::Simulation(std::shared_ptr<Grid> grid, const PresenceManager& pm,
             std::shared_ptr<SocialMixingScoreProvider> scoring,
             std::shared_ptr<TransmissionModelProvider> transmission)
 : m_grid(grid), m_pm(pm), 
   scoreProvider(scoring), modelProvider(transmission),
   maxTicks(0), minutesPerTick(1.0), currentTick(0), today(0), casesPerDay(0), recoveredPerDay(0),
   rd(), gen(rd()), distrib(-1,1)
{
  ;
}


void
Simulation::runToCompletion(uint64_t days, uint64_t secondsPerTick)
{
  reset(days, secondsPerTick);
  for (uint64_t ticks = 0;ticks < maxTicks;ticks++) {
    tick();
  }
}

void
Simulation::runToCompletion(uint64_t days, uint64_t secondsPerTick, 
  std::shared_ptr<IntermediateResultsListener> callback, uint64_t ticksPerCallback) 
{
  reset(days, secondsPerTick);
  uint64_t lastCbTicks = 0;
  // NOTE: index 0 is the initial state, NOT the state after the first tick
  for (uint64_t ticks = 0;ticks <= maxTicks;ticks++) {
    if (0 == (ticks % ticksPerCallback)) {
      callback->intermediateResults(casesPerDay[today],recoveredPerDay[today],m_pm,
        currentTick == 0 ? 0 : ticksPerCallback * minutesPerTick,
        currentTick);
      lastCbTicks = ticks;
    }
    tick();
  }
  // don't forget final callback
  callback->intermediateResults(casesPerDay[today],recoveredPerDay[today],m_pm,
     (maxTicks - lastCbTicks)*minutesPerTick,
    maxTicks);
}

bool
Simulation::writeStandardResults(std::string outputFolder) noexcept
{
  // TODO settings

  // final state
  uint64_t totalInfectedEver = 0;
  double totalHighestRiskScoreIll = 0.0;
  double totalHighestRiskScoreNotIll = 0.0;
  for (uint64_t id = 0;id < m_pm.size();id++) {
    auto p = m_pm.get(id);
    if (p->hasEverBeenIll()) {
      totalInfectedEver++;
      totalHighestRiskScoreIll += p->highestRiskScore();
    } else {
      totalHighestRiskScoreNotIll += p->highestRiskScore();
    }
  }
  double pctNotIll = 0.0;
  std::cout << "Total people infected: " << totalInfectedEver 
            << " (" << (100.0 * totalInfectedEver / m_pm.size()) << "%)"
            << std :: endl;
  if (0 == totalInfectedEver) {
    std::cout << "  Avg risk score for those who did fall ill    : N/A (No one fell ill)"
              << std::endl;
  } else {
    std::cout << "  Avg risk score for those who did fall ill    : " << totalHighestRiskScoreIll / totalInfectedEver
              << std::endl;
  }
  if (m_pm.size() == totalInfectedEver) {
    std::cout << "  Avg risk score for those who did not fall ill: N/A (All were ill)"
              << std::endl;
  } else {
    std::cout << "  Avg risk score for those who did not fall ill: " << totalHighestRiskScoreNotIll / (m_pm.size() - totalInfectedEver)
              << std::endl;
  }
  return true;
}

// PRIVATE METHODS

void
Simulation::reset(uint64_t days, uint64_t secondsPerTick)
{
  currentTick = 0;
  today = 0;
  maxTicks = (uint64_t)std::ceil(days * ((60.0 / secondsPerTick) * 60 * 24));
  std::cout << "SETTING: maxTicks = " << maxTicks << std::endl;
  
  // now place them
  m_grid->randomisePositions(m_pm);

  // now initialise risk
  std::cout << "Infecting... ";
  for (uint64_t id = 0;id < m_pm.size();id++) {
    std::shared_ptr<Presence> p = m_pm.get(id);
    scoreProvider->initialiseRiskScore(p);
    modelProvider->initialiseInfectionState(p);
    p->commitChanges();
  }
  std::cout << std::endl;

  // sanity check - ensure right number of people are infected
  uint64_t infectedCheck = 0;
  for (uint64_t id = 0;id < m_pm.size();id++) {
    std::shared_ptr<Presence> p = m_pm.get(id);
    if (p->state() == State::Ill) {
      infectedCheck++;
    }
  }
  std::cout << "CHECK: Infected - count: " << infectedCheck << std::endl;

  casesPerDay.clear();
  recoveredPerDay.clear();
  casesPerDay.push_back(infectedCheck);
  recoveredPerDay.push_back(0);
}

void
Simulation::tick()
{
  if (0 == currentTick % 100) {
    std::cout << "Current Tick: " << currentTick << std::endl;
  }
  // calculate any movements in position
  for (uint64_t id = 0;id < m_pm.size();id++) {
    auto actor = m_pm.get(id);
    uint64_t newX = actor->position()->x() + distrib(gen);
    uint64_t newY = actor->position()->y() + distrib(gen);
    if (newX < 0) {
      newX = 0;
    } else if (newX >= m_grid->width()) {
      newX = m_grid->width() - 1;
    }
    if (newY < 0) {
      newY = 0;
    } else if (newY >= m_grid->height()) {
      newY = m_grid->height() - 1;
    }
    actor->moveTo(m_grid->cell(newX,newY));
  }

  // calculate new social mixing risk score
  for (uint64_t id = 0;id < m_pm.size();id++) {
    auto actor = m_pm.get(id);
    scoreProvider->calculateNewRiskScore(actor,minutesPerTick);
  }
  // calculate actual medical state
  for (uint64_t id = 0;id < m_pm.size();id++) {
    auto actor = m_pm.get(id);
    modelProvider->determineInfectionState(actor,minutesPerTick, currentTick);
  }
  // Commit new risk score (two step process in case of nearby over more than 1 grid square)
  for (uint64_t id = 0;id < m_pm.size();id++) {
    auto actor = m_pm.get(id);
    actor->commitChanges();
  }
  // increment tick
  currentTick++;
  uint64_t newToday = (uint64_t)(currentTick * minutesPerTick) / (60 * 24);
  if (newToday > today) {
    // recalculate cases
    uint64_t liveCases = 0;
    uint64_t liveRecovered = 0;
    for (uint64_t id = 0;id < m_pm.size();id++) {
      auto actor = m_pm.get(id);
      switch (actor->state()) {
        case State::Ill:
          liveCases++;
          break;
        case State::Recovered:
          liveRecovered++;
          break;
        default:
          break;
      }
    }
    casesPerDay.push_back(liveCases);
    recoveredPerDay.push_back(liveRecovered);
  }
  today = newToday;
}


}
}