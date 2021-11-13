//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_RISK_SCORE_H
#define HERALD_RISK_SCORE_H

/**
 * /brief Represents an estimated Risk score through calculation, typically from a mix of factors
 *        including data about an individual, and exposures from the environment measured on device.
 *
 * This is different from an exposure in that it uses exposure data which may be supplied continuously
 * for short periods of time per exposure value, and calculates a risk score from these source variables.
 * There may be many risk score algorithms that run over a different combination of variables. This struct
 * represents the result (which may be comparable to results from other risk score algorithms) and not
 * the metadata on how the score was calculated. That is held in the generating algorithm class, and
 * managed by the Risk Manager.
 */
struct RiskScore {
  UUID agentId = UUID::unknown();
  Date periodStart; // defaults to "now"
  Date periodEnd; // defaults to "now"
  double value = 0.0;
  double confidence = 1.0;
};

#endif