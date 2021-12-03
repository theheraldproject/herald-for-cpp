//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ANALYSIS_SAMPLE_ALGORITHMS_H
#define HERALD_ANALYSIS_SAMPLE_ALGORITHMS_H

#include <cmath>

#include "aggregates.h"
#include "ranges.h"
#include "runner.h"
#include "sampling.h"
#include "../datatype/rssi_minute.h"
#include "../datatype/rssi.h"
#include "../datatype/time_interval.h"
#include "../datatype/derived.h"

// Debug only
// #include <iostream>

namespace herald {
namespace analysis {
namespace algorithms {

using namespace herald::analysis::aggregates;
using namespace herald::analysis::sampling;
using namespace herald::datatype;

struct MeanValidRSSI {
  static constexpr int runs = 1;

  MeanValidRSSI() : run(1), value(0), count(0) {}
  ~MeanValidRSSI() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
  }

  template <typename ValT>
  void map(ValT newValue) {
    double tv = 100.0 + (double)newValue; // rssi is a negative
    if (tv < 0) {
      return;
    }
    if (tv > 100) {
      return;
    }
    value += tv;
    ++count;
  }

  double reduce() {
    if (0 == count) {
      return 0;
    }
    return value / count;
  }

  void reset() {
    run = 1;
    value = 0;
    count = 0;
  }

  int getCount() const noexcept {
    return count;
  }

private:
  int run;
  double value;
  int count;
};

struct RSSIMinutesAnalyser {
  using input_value_type = RSSI;
  using output_value_type = RSSIMinute;
  // static constexpr std::size_t classId = RSSIMinute::classId;

  /// default constructor required for array instantiation in manager AnalysisProviderManager
  RSSIMinutesAnalyser() : interval(5), calculator(), lastRan(0), hasRan(false) {}
  RSSIMinutesAnalyser(long interval) : interval(interval), calculator(), lastRan(0), hasRan(false) {}
  ~RSSIMinutesAnalyser() = default;

  // Generic
  // TODO consider removing this annoyance somehow...
  template <typename SrcT, std::size_t SrcSz,typename DstT, std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<SrcT>,SrcSz>& src, SampleList<Sample<DstT>,DstSz>& dst, CallableForNewSample& callable) {
    return false; // no op - compiled out
  }

  // Specialisation
  template <std::size_t SrcSz,std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<RSSI>,SrcSz>& src, SampleList<Sample<RSSIMinute>,DstSz>& dst, CallableForNewSample& callable) {
    if (lastRan + interval >= timeNow) {
      return false; // interval guard.
    }
    // std::cout << "RUNNING FOWLER BASIC ANALYSIS at " << timeNow.secondsSinceUnixEpoch() << std::endl;

    // split into windows of data based on interval time
    Date startInterval = lastRan;
    while (startInterval <= timeNow) {
      // limit also to before startInterval + interval
      herald::analysis::views::beforeOrEqual beforeEndOfThisInterval(startInterval + interval);

      herald::analysis::views::in_range valid(-99,-10);

      // Check that there has been any new data since the last run
      herald::analysis::views::since sinceLastRun(lastRan);
      auto newData = src
                  | herald::analysis::views::filter(valid) 
                  | herald::analysis::views::filter(beforeEndOfThisInterval)
                  | herald::analysis::views::filter(sinceLastRun)
                  | herald::analysis::views::to_view();

      calculator.reset();

      auto values = src 
                  | herald::analysis::views::filter(valid) 
                  | herald::analysis::views::filter(beforeEndOfThisInterval)
                  | herald::analysis::views::filter(sinceLastRun)
                  | aggregate(calculator); // type is MeanValidRSSI
      
      auto agg = values.template get<MeanValidRSSI>();
      if (!hasRan) {
        // use first time in sequence as lastRan for this purpose
        Date firstTime = src.earliest();
        startInterval = firstTime;
        hasRan = true;
      }
      if (agg.getCount() > 0) { // only output a sample if we have data in this check window
        auto d = agg.reduce();

        Date latestTime = startInterval + interval;
        // latest WITHIN our interval query
        if (latestTime > src.latest()) {
          latestTime = src.latest();
        }
        TimeInterval timeDelta = latestTime - startInterval;
        // startInterval = latestTime;
        // std::cout << "Latest value at time: " << latestTime.secondsSinceUnixEpoch() << std::endl;

        Sample<RSSIMinute> newSample((Date)latestTime,RSSIMinute(d * timeDelta / (1000 * 60.0))); // timeDelta is millis!
        dst.push(newSample);

        // fire event
        callable(sampled,newSample);
      }
      // Now move our time window forward
      startInterval += interval;
      lastRan = startInterval; // TODO move this logic to the caller not the analysis provider
    }
    lastRan = timeNow;
    return true;
  }

private:
  TimeInterval interval;
  MeanValidRSSI calculator;
  Date lastRan;
  bool hasRan;
};

/**
 * @brief Calculates a running mean over an arbitrary number of samples, but only over the MaxRecentValues number of most recent values.
 * 
 * @tparam ValT The type to calculate the running mean over
 * @tparam MaxRecentValues The number of most recent values (max - could be less) 
 */
template <typename ValT, std::size_t MaxRecentValues>
struct RunningMeanAggregate {
  static constexpr int runs = 1;

  RunningMeanAggregate() : run(1), values() {}
  RunningMeanAggregate(const RunningMeanAggregate<ValT,MaxRecentValues>& other) : run(other.run), values() {
    // for (const auto& v: other.values) {
    //   values.add(v);
    // }
  }
  RunningMeanAggregate(RunningMeanAggregate<ValT,MaxRecentValues>&& other) : run(other.run), values() {
  //   for (auto& v: other.values) {
  //     values.add(v);
  //   }
  }
  ~RunningMeanAggregate() = default;

  // Copy assign and move assign operators
  RunningMeanAggregate& operator=(const RunningMeanAggregate& other) noexcept {
    run = other.run;
    return *this;
  }
  RunningMeanAggregate& operator=(RunningMeanAggregate&& other) noexcept {
    run = other.run;
    return *this;
  }

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
  }

  template <typename MapValT>
  void map(MapValT newValue) {
    values.push(newValue);
  }

  double reduce() {
    if (0 == values.size()) {
      return 0;
    }
    double running = 0;
    for (auto& v : values) {
      running += (double)v;
    }
    return running / values.size();
  }

  void reset() {
    run = 1;
    values.clear();
  }

  int getCount() const noexcept {
    return values.size();
  }

private:
  int run;
  SampleList<Sample<ValT>,MaxRecentValues> values;
};



template <typename ValT, std::size_t MaxRecentValues>
struct RunningMeanAnalyser {
  using input_value_type = ValT;
  using output_value_type = RunningMean<ValT>;

  static constexpr std::size_t max_size = MaxRecentValues;

  /// default constructor required for array instantiation in manager AnalysisProviderManager
  RunningMeanAnalyser() = default;
  ~RunningMeanAnalyser() = default;

  // Generic
  // TODO consider removing this annoyance somehow...
  template <typename SrcT, std::size_t SrcSz,typename DstT, std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<SrcT>,SrcSz>& src, SampleList<Sample<DstT>,DstSz>& dst, CallableForNewSample& callable) {
    return false; // no op - compiled out
  }

  // Specialisation
  template <std::size_t SrcSz,std::size_t DstSz, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<ValT>,SrcSz>& src, SampleList<Sample<RunningMean<ValT>>,DstSz>& dst, CallableForNewSample& callable) {

      recentMean.reset();

      auto values = src 
                  | aggregate(recentMean); // type is RunningMean<ValT>
      
      auto agg = values.template get<RunningMeanAggregate<ValT,MaxRecentValues>>();
      if (agg.getCount() > 0) { // only output a sample if we have data
        auto d = agg.reduce();

        Sample<RunningMean<ValT>> newSample((Date)src.latest(),RunningMean<ValT>(d));
        dst.push(newSample);

        // fire event
        callable(sampled,newSample);
        return true;
      }
    return false;
  }

private:
  RunningMeanAggregate<ValT, MaxRecentValues> recentMean;
};

}
}
}

#endif
