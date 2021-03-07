//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef RISK_H
#define RISK_H

#include <cmath>

#include "aggregates.h"
#include "ranges.h"
#include "sampling.h"

namespace herald {
namespace analysis {
namespace algorithms {
namespace risk {

using namespace herald::analysis::aggregates;

struct RiskAggregationBasic {
  static constexpr int runs = 1;

  RiskAggregationBasic(double timeScale,double distanceScale,double minimumDistanceClamp,double minimumRiskScoreAtClamp,double logScale = 3.3598856662 ) 
    : run(1), timeScale(timeScale), distanceScale(distanceScale), minimumDistanceClamp(minimumDistanceClamp), 
      minimumRiskScoreAtClamp(minimumRiskScoreAtClamp), logScale(logScale), nMinusOne(-1.0), n(-1.0), timeMinusOne(0), time(0)
  {
    ; // no other set up
  }
  ~RiskAggregationBasic() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
    if (1 == run) {
      // clear run temporaries
      nMinusOne = -1.0;
      n = -1.0;
      timeMinusOne = 0;
      time = 0;
    }
  }

  template <typename ValT,
            typename SampleT = typename ValT::value_type
           >
  void map(ValT value) {
    using T = std::decay<ValT>;
    if constexpr (std::is_same_v<T,herald::analysis::sampling::Sample<SampleT>>) { // Note: May need to be Sample<double>
      // is a valid type for evaluation
      nMinusOne = n;
      timeMinusOne = time;
      n = value.value;
      time = value.taken;
    }
  }

  double reduce() {
    if (-1.0 != nMinusOne) {
      // we have two values with which to calculate
      // using nMinusOne and n, and calculate interim risk score addition
      double dist = distanceScale * n;
      double t = timeScale * (time - timeMinusOne); // seconds

      double riskSlice = minimumRiskScoreAtClamp; // assume < clamp distance
      if (dist > minimumDistanceClamp) {
        // otherwise, do the inverse log of distance to get the risk score

        // don't forget to clamp at risk score
        riskSlice = minimumRiskScoreAtClamp - (logScale * std::log10(dist));
        if (riskSlice > minimumRiskScoreAtClamp) {
          // possible as the passed in logScale could be a negative
          riskSlice = minimumRiskScoreAtClamp;
        }
        if (riskSlice < 0.0) {
          riskSlice = 0.0; // cannot have a negative slice
        }
      }
      riskSlice *= t;
      
      // add it to the risk score
      riskScore += riskSlice;
    }

    // return current full risk score
    return riskScore;
  }

  void reset() {
    run = 1;
    riskScore = 0.0;
    nMinusOne = -1.0;
    n = -1.0;
  }

private:
  int run;

  double timeScale;
  double distanceScale;
  double minimumDistanceClamp;
  double minimumRiskScoreAtClamp;
  double logScale;

  double nMinusOne; // distance of n-1
  double n; // distance of n
  long timeMinusOne; // time of n-1
  long time; // time of n

  double riskScore;
};

}
}
}
}

#endif
