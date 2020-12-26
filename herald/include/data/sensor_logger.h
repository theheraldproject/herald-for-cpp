//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_LOGGER_H
#define SENSOR_LOGGER_H

#include "../datatype/bluetooth_state.h"
#include "../context.h"

#include "../../fmt/include/fmt/format.h"

#include <string>
#include <memory>
#include <ostream>

namespace herald {
namespace data {

// TODO is the below used anywhere?
enum class SensorLoggerLevel : int {
  debug, info, fault
};

// NOTE: HEADER ONLY CLASS AS IT USES VARIABLE TEMPLATE ARGS FOR LOGGING

class SensorLogger {
public:
  SensorLogger(const std::shared_ptr<Context>& ctx, std::string subsystem, std::string category) 
    : mSink(ctx->getLoggingSink("SensorLogger")), mSubsystem(subsystem), mCategory(category)
  {
    ;
  }

  virtual ~SensorLogger() = default; //  TODO is this valid to close mSink?

  // use std::format to generate the string
  // std::format in C++20, fmt::format library before that
  // Note: C++11 Variadic template parameter pack expansion
  template <typename ... Types>
  void debug(const std::string& message, const Types&... args) {
    std::string msg =  fmt::format(message,args...);
    log(sDebug, msg);
  }

  template <typename ... Types>
  void info(const std::string& message, const Types&... args) {
    std::string msg =  fmt::format(message,args...);
    log(sInfo, msg);
  }

  template <typename ... Types>
  void fault(const std::string& message, const Types&... args) {
    std::string msg =  fmt::format(message,args...);
    log(sFault, msg);
  }

private:
  inline void log(std::string lvl, std::string msg) {
    // TODO csv string encapsulate msg
    // TODO include timestamp
    mSink << lvl << mSubsystem << sComma << mCategory << sComma << msg << std::endl;
  }

  std::ostream& mSink;
  std::string mSubsystem;
  std::string mCategory;

  static std::string sComma;
  static std::string sDebug;
  static std::string sInfo;
  static std::string sFault;
};

std::string SensorLogger::sComma = ",";
std::string SensorLogger::sDebug = ",debug,";
std::string SensorLogger::sInfo  = ",info,";
std::string SensorLogger::sFault = ",fault,";

} // end namespace
} // end namespace

#endif