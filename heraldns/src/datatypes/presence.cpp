//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "../../heraldns.h"

#include <iostream>
#include <random>

using namespace heraldns;

namespace heraldns {
namespace datatype {


Presence::Presence(std::uint64_t id)
  : m_id(id), m_currentRisk(0), m_newRisk(0), m_flightiness(0.0),
    m_position(), m_state(State::Well), m_transmissionModelScore(0.0), m_newTransmissionModelScore(0.0),
    m_lastFellIll(0), m_lastRecovered(0), m_hasEverBeenIll(false), m_highestRiskScore(0.0),
    m_currentTransmittedRisk(0), m_newTransmittedRisk(0)
{
  ;
}

std::shared_ptr<Cell>
Presence::position() const
{
  return *m_position;
}

void
Presence::moveTo(std::shared_ptr<Cell> newPlace)
{
  if (m_position) {
    (*m_position)->movedOut(m_id); // TODO verify this does what we think
  }
  m_position = newPlace;
  newPlace->movedIn(m_id);
}

double
Presence::risk() const
{
  return m_currentRisk;
}

// Calculated Risk Score using the current formula
double
Presence::newRisk()
{
  return m_newRisk;
}

void
Presence::newRisk(double newRisk)
{
  m_newRisk = newRisk;
}

// Calculated transmission risk number using the current formula

// Cumulative risk-minutes exposed to
void
Presence::newTransmittedRisk(double riskMinutes)
{
  m_newTransmittedRisk = riskMinutes;
}

double
Presence::newTransmittedRisk() const
{
  return m_newTransmittedRisk;
}

double
Presence::transmittedRisk() const
{
  return m_currentTransmittedRisk;
}

// Person-specific functions
void
Presence::flightiness(double flightiness)
{
  m_flightiness = flightiness;
}

double
Presence::flightiness()
{
  return m_flightiness;
}

void
Presence::commitChanges()
{
  m_currentRisk = m_newRisk;
  m_newRisk = 0.0;
  // set newRisk to 0
  // If the caller needs it to be the same as currentRisk then it can do this
  // but if we do that here then it is impossible for a caller to tell.
  m_currentTransmittedRisk = m_newTransmittedRisk;
  // We know transmitted amount from previous ticks always degrades over time, so don't reset it here
  m_state = m_newState;
  if (m_currentRisk > m_highestRiskScore) {
    m_highestRiskScore = m_currentRisk;
  }
}

State
Presence::state() const
{
  return m_state;
}

void
Presence::newState(State newState, uint64_t atTick)
{
  if (m_state == State::Well && newState == State::Ill) {
    m_lastFellIll = atTick;
    m_hasEverBeenIll = true;
  } else if (m_state == State::Ill && newState == State::Recovered) {
    m_lastRecovered = atTick;
    m_newTransmittedRisk = 0.0;
    m_newTransmissionModelScore = 0.0; // reset and try to become ill again!
  }
  // m_state changed at commitChanges() only
  m_newState = newState;
}

uint64_t
Presence::id() const
{
  return m_id;
}

uint64_t
Presence::lastFellIll() const
{
  return m_lastFellIll;
}

bool
Presence::hasEverBeenIll() const
{
  return m_hasEverBeenIll;
}

uint64_t
Presence::lastRecovered() const
{
  return m_lastRecovered;
}

double
Presence::highestRiskScore() const
{
  return m_highestRiskScore;
}


double
Presence::transmissionModelScore() const
{
  return m_transmissionModelScore;
}

double
Presence::newTransmissionModelScore() const
{
  return m_newTransmissionModelScore;
}

void
Presence::newTransmissionModelScore(double riskMinutes)
{
  m_newTransmissionModelScore = riskMinutes;
}




PresenceManager::PresenceManager(uint64_t count)
  : actors()
{
  actors.reserve(count);
  for (uint64_t id = 0;id < count;id++) {
    actors.push_back(std::make_shared<Presence>(id));
  }
}

uint64_t
PresenceManager::size() const
{
  return actors.size();
}

std::shared_ptr<Presence>
PresenceManager::get(uint64_t id) const
{
  return actors[id];
}


}
}
