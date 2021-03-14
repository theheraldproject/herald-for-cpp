//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DISTANCE_CONVERSION_H
#define DISTANCE_CONVERSION_H

#include <cmath>

#include "aggregates.h"
#include "ranges.h"
#include "runner.h"
#include "sampling.h"
#include "../datatype/distance.h"

namespace herald {
namespace analysis {
namespace algorithms {
namespace distance {

using namespace herald::analysis::aggregates;
using namespace herald::analysis::sampling;
using namespace herald::datatype;

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

struct FowlerBasicAnalyser {
  FowlerBasicAnalyser(long interval, double intercept, double coefficient) : interval(interval), basic(intercept, coefficient), lastRan(0) {}
  ~FowlerBasicAnalyser() = default;

  template <std::size_t SrcSz>
  void analyse(Date timeNow, const SampleList<Sample<RSSI>,SrcSz>& src, analysis::AnalysisDelegate<Distance>& dst) {
    if (lastRan + interval > timeNow) return; // interval guard

    basic.reset();

    herald::analysis::views::in_range valid(-99,-10);

    auto values = src 
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::to_view();

    auto summary = values
                 | summarise<Mode,Variance>();

    auto mode = summary.get<Mode>();
    auto var = summary.get<Variance>();
    auto sd = std::sqrt(var);

    auto distance = sl 
                  | herald::analysis::views::filter(valid) 
                  | herald::analysis::views::filter(
                      herald::analysis::views::in_range(
                        mode - 2*sd, // NOTE: WE USE THE MODE FOR FILTER, BUT SD FOR BOUNDS - See website for the reasoning
                        mode + 2*sd
                      )
                    )
                  | aggregate(basic); // type actually <herald::analysis::algorithms::distance::FowlerBasic>
    
    auto agg = distance.get<FowlerBasic>();
    auto d = agg.reduce();

    Date latestTime = values.latest();

    dst.newSample(Sample<Distance>(latestTime,Distance(d)));
  }

private:
  long interval;
  FowlerBasic basic;
  Date lastRan;
};

}
}
}
}

#endif
