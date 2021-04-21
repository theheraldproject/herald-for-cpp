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

template <typename ContextT>
struct OptionalSensorLogger {
  OptionalSensorLogger(ContextT& ctx) 
    : m_context(ctx)
      HLOGGERINIT(m_context,"herald","LoggingAnalysisDelegate")
  {
    ;
  }

  OptionalSensorLogger(const OptionalSensorLogger& other)
    : m_context(other.m_context)
      HLOGGERINIT(m_context,"herald","LoggingAnalysisDelegate")
  {
    ;
  }

  OptionalSensorLogger(OptionalSensorLogger&& other)
    : m_context(other.m_context)
      HLOGGERINIT(m_context,"herald","LoggingAnalysisDelegate")
  {
    ;
  }

  OptionalSensorLogger& operator=(OptionalSensorLogger&& other)
  {
    m_context = other.m_context;
    logger = other.logger;
    return *this;
  }

  OptionalSensorLogger& operator=(const OptionalSensorLogger& other)
  {
    m_context = other.m_context;
    logger = other.logger;
    return *this;
  }

  void debug(std::string toLog,SampledID sampled,double value)
  {
    HTDBG(toLog);
    // HTDBG(std::to_string(sampled));
    // HTDBG(std::to_string(value));
  }

  void debug(std::string toLog)
  {
    HTDBG(toLog);
  }

private:
  ContextT& m_context;
  HLOGGER(ContextT);
};

/// \brief Logs any given type of sample to the Herald logging subsystem as a Debug message
template <typename ContextT, typename ValT>
struct LoggingAnalysisDelegate {
  using value_type = ValT;
  
  LoggingAnalysisDelegate()
    : ctx(),
      logger()
  {
    ;
  }

  LoggingAnalysisDelegate(ContextT& context)
    : ctx(context),
      logger(ctx)
  {
    ;
  }

  LoggingAnalysisDelegate(const LoggingAnalysisDelegate& other) noexcept
    : ctx(other.ctx),
      logger(ctx)
  {
    ;
  }

  LoggingAnalysisDelegate(LoggingAnalysisDelegate&& other) noexcept
    : ctx(other.ctx),
      logger(ctx)
  {
    ;
  }
  
  ~LoggingAnalysisDelegate() = default;

  LoggingAnalysisDelegate& operator=(LoggingAnalysisDelegate&& other) noexcept {
    ctx = other.ctx;
    logger = other.logger;
    return *this;
  }

  void assignContext(ContextT& newContext) noexcept {
    ctx = newContext;
    logger.emplace(ctx);
  }

  // specific override of template
  void newSample(SampledID sampled, Sample<ValT> sample) {
    // Log the read distance as debug
    if (!logger.has_value()) { // GUARD
      return;
    }
    logger.value().debug("New Sample Recorded.");
    // logger.value().debug("New Sample Recorded. SampledID: {}, Value: {}",sampled,(double)sample);
  }

private:
  std::optional<std::reference_wrapper<ContextT>> ctx;
  std::optional<OptionalSensorLogger<ContextT>> logger;
};

}
}

#endif