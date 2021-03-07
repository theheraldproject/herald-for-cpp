//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DISTANCE_H
#define DISTANCE_H

#include <cmath>

#include "aggregates.h"
#include "ranges.h"

namespace herald {
namespace analysis {
namespace algorithms {
namespace distance {

using namespace herald::analysis::aggregates;

struct FowlerBasic {
  static constexpr int runs = 1;

  FowlerBasic(double intercept, double coefficient) : run(1), mode(), intercept(intercept), coefficient(coefficient) {}
  ~FowlerBasic() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
    mode.beginRun(thisRun);
  }

  template <typename ValT>
  void map(ValT value) {
    mode.map(value);
  }

  double reduce() {
    double exponent = (mode.reduce() - intercept) / coefficient;
    return std::pow(10, exponent); // distance
  }

  void reset() {
    run = 1;
    mode.reset();
  }

private:
  int run;
  Mode mode; // cleaner to use the Mode rather than redo it in this class
  double intercept;
  double coefficient;
};

}
}
}
}

#endif
