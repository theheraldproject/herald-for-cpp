//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DISTANCE_CONVERSION_H
#define HERALD_DISTANCE_CONVERSION_H

#include <cmath>

#include "aggregates.h"
#include "ranges.h"
#include "runner.h"
#include "sampling.h"
#include "../datatype/distance.h"

// Debug only
// #include <iostream>

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
  using input_value_type = RSSI;
  using output_value_type = Distance;

  /// default constructor required for array instantiation in manager AnalysisProviderManager
  FowlerBasicAnalyser() : interval(10), basic(-11,-0.4), lastRan(0) {}
  FowlerBasicAnalyser(long interval, double intercept, double coefficient) : interval(interval), basic(intercept, coefficient), lastRan(0) {}
  ~FowlerBasicAnalyser() = default;

  // Generic
  // TODO consider removing this annoyance somehow...
  template <typename SrcT, std::size_t SrcSz,typename DstT, std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<SrcT>,SrcSz>& src, SampleList<Sample<DstT>,DstSz>& dst, CallableForNewSample& callable) {
    return false; // no op - compiled out
  }

  // Specialisation
  template <std::size_t SrcSz,std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<RSSI>,SrcSz>& src, SampleList<Sample<Distance>,DstSz>& dst, CallableForNewSample& callable) {
    if (lastRan + interval >= timeNow) return false; // interval guard
    // std::cout << "RUNNING FOWLER BASIC ANALYSIS at " << timeNow.secondsSinceUnixEpoch() << std::endl;

    basic.reset();

    herald::analysis::views::in_range valid(-99,-10);

    auto values = src 
                | herald::analysis::views::filter(valid) 
                | herald::analysis::views::to_view();

    auto summary = values
                 | summarise<Mode,Variance>();

    auto mode = summary.template get<Mode>();
    auto var = summary.template get<Variance>();
    auto sd = std::sqrt(var);

    auto distance = src 
                  | herald::analysis::views::filter(valid) 
                  | herald::analysis::views::filter(
                      herald::analysis::views::in_range(
                        mode - 2*sd, // NOTE: WE USE THE MODE FOR FILTER, BUT SD FOR BOUNDS - See website for the reasoning
                        mode + 2*sd
                      )
                    )
                  | aggregate(basic); // type actually <herald::analysis::algorithms::distance::FowlerBasic>
    
    auto agg = distance.template get<FowlerBasic>();
    auto d = agg.reduce();


    Date latestTime = values.latest();
    lastRan = latestTime; // TODO move this logic to the caller not the analysis provider
    // std::cout << "Latest value at time: " << latestTime.secondsSinceUnixEpoch() << std::endl;

    Sample<Distance> newSample((Date)latestTime,Distance(d));
    dst.push(newSample);
    callable(sampled,newSample);
    return true;
  }

private:
  TimeInterval interval;
  FowlerBasic basic;
  Date lastRan;
};

}
}
}
}

#endif
