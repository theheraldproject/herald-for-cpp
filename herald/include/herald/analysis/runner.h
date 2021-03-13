//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ANALYSIS_RUNNER_H
#define ANALYSIS_RUNNER_H

#include <variant>
#include <array>

namespace herald {
namespace analysis {

template <typename... AnalyserT>
class AnalysisRunner {
public:
  static constexpr Size = sizeof...(AnalyserT);

  AnalysisRunner(AnalyserT... analyserList) : analysers() {
    analysers.push(std::move(analyserList...));
  }
  ~AnalysisRunner() = default;

  // Public methods here
  void add(DistanceDelegate delegate) {

  }

  void run() {

  }

  template <typename SampleT>
  SampleList<SampleT>&

private:
  std::array<std::variant<AnalyserT>,Size> analysers;
};

}
}

#endif