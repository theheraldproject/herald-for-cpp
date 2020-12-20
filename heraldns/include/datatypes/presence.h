//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//


#ifndef PRESENCE_H
#define PRESENCE_H

#include "grid.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <optional>

namespace heraldns {
namespace datatype {

enum class State : int {
  Well, Ill, Recovered, Dead
};

/**
 * Base class for people, static items, etc.
 */
class Presence {
public:
  Presence(std::uint64_t id);
  virtual ~Presence() = default;

  // BEHAVIOUR SETTINGS
  void flightiness(double flightiness); // How likely it is that a person generally moves about
  double flightiness();


  // ACTUAL STATE TRACKING

  uint64_t id() const;

  uint64_t lastFellIll() const; // 0 if never
  bool hasEverBeenIll() const;
  double highestRiskScore() const;
  uint64_t lastRecovered() const;

  State state() const;
  void newState(State newState, uint64_t atTick);

  std::shared_ptr<Cell> position() const;
  void moveTo(std::shared_ptr<Cell> newPlace); // will remove presence from current cell (will call cell function to ensure this), then will move to the newPlace.



  // ACTUAL TRANSMISSION MODEL TRACKING
  double transmissionModelScore() const;
  // For *actual* illness tracking exposed to
  double newTransmissionModelScore() const;
  void newTransmissionModelScore(double riskMinutes);


  // SOCIAL MIXING SCORE APPROXIMATION SCORING
  double risk() const; // Current risk - 0-1

  // Calculated Risk Score using the current formula
  double newRisk(); // current risk prior to commital - 0-1
  void newRisk(double newRisk); // modify new risk prior to committal - 0-1

  // Calculated risk number the social mixing function may transmit to other phones 
  double transmittedRisk() const;

  double newTransmittedRisk() const;
  void newTransmittedRisk(double newT);

  // STATE CHANGES
  void commitChanges(); // Move 'newRisk' to 'Risk' (at end of this sim 'turn')

private:
  const uint64_t m_id;
  
  double m_currentRisk;
  double m_newRisk;

  double m_currentTransmittedRisk;
  double m_newTransmittedRisk;

  double m_flightiness;

  std::optional<std::shared_ptr<Cell>> m_position;

  State m_state;
  State m_newState;

  double m_transmissionModelScore;
  double m_newTransmissionModelScore;

  // current state metrics
  uint64_t m_lastFellIll;
  uint64_t m_lastRecovered;

  // for all time metrics
  bool m_hasEverBeenIll;
  double m_highestRiskScore;
};

class PresenceManager {
public:
  PresenceManager(uint64_t count);
  ~PresenceManager() = default;

  uint64_t size() const;
  std::shared_ptr<Presence> get(uint64_t id) const;

private:
  std::vector<std::shared_ptr<Presence>> actors;
};

} // end namespace
} // end namespace

#endif