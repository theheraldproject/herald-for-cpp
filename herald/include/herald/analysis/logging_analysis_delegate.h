//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_LOGGING_ANALYSIS_DELEGATE_H
#define HERALD_LOGGING_ANALYSIS_DELEGATE_H

#include "sampling.h"
#include "../data/sensor_logger.h"
#include "../context.h"

#include <memory>
#include <optional>
#include <functional>

namespace herald {
namespace analysis {

using namespace sampling;

/// \brief Logs any given type of sample to the Herald logging subsystem as a Debug message
template <typename ContextT, typename ValT>
struct LoggingAnalysisDelegate {

  using value_type = ValT;
  
  // LoggingAnalysisDelegate() 
  //   : ctx(nullptr)
  //   HLOGGERINIT(nullptr,"herald","LoggingAnalysisDelegate")
  // {
  // }
  LoggingAnalysisDelegate()
    : ctx()
    // HLOGGERINIT(ctx,"herald","LoggingAnalysisDelegate")
  {
  }

  LoggingAnalysisDelegate(ContextT& context)
    : ctx(context)
    HLOGGERINIT(ctx.value().get(),"herald","LoggingAnalysisDelegate")
  {
  }

  LoggingAnalysisDelegate(const LoggingAnalysisDelegate&) = delete; // copy ctor deleted
  LoggingAnalysisDelegate(LoggingAnalysisDelegate&& other) noexcept
    : ctx(other.ctx)
    HLOGGERINIT(ctx.value().get(),"herald","LoggingAnalysisDelegate")
  {
  } // move ctor
  
  ~LoggingAnalysisDelegate() {
  }

  LoggingAnalysisDelegate& operator=(LoggingAnalysisDelegate&& other) noexcept {
    ctx = other.ctx;
    // TODO assign logger too (may have been default initialised by the Analysis engine)
    return *this;
  }

  // specific override of template
  void newSample(SampledID sampled, Sample<ValT> sample) {
    // Log the read distance as debug
    HTDBG("New Sample Recorded"); // TODO include the sampled id, sample time, sample value
  }

private:
  std::optional<std::reference_wrapper<ContextT>> ctx;
  HLOGGER(ContextT);

};

}
}

#endif