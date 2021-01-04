//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_LOGGER_H
#define SENSOR_LOGGER_H

#include "../datatype/bluetooth_state.h"
#include "../context.h"

// #include "fmt/format.h"

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

class SensorLoggingSink {
public:
  SensorLoggingSink() = default;
  virtual ~SensorLoggingSink() = default;

  virtual void log(SensorLoggerLevel level, std::string message) = 0;
};

class SensorLogger {
public:
  SensorLogger(const std::shared_ptr<Context>& ctx, std::string subsystem, std::string category) 
    : mSink(ctx->getLoggingSink(subsystem, category)), mSubsystem(subsystem), mCategory(category)
  {
    ;
  }
  
  // TODO consider supporting multiple sinks in the context - E.g. USB UART and log file

  ~SensorLogger() {}; // define this ourselves, but blank and not default

  // use std::format to generate the string
  // std::format in C++20, fmt::format library before that
  // Note: C++11 Variadic template parameter pack expansion
  template <typename ... Types>
  void debug(const std::string& message, const Types&... args) {
    // std::string msg =  fmt::format(message,args...);
    char buffer[256];
    int len = snprintf(buffer, 256, message.c_str(), args...);
    std::string msg(buffer,len);
    log(SensorLoggerLevel::debug, msg);
  }

  template <typename ... Types>
  void info(const std::string& message, const Types&... args) {
    // std::string msg =  fmt::format(message,args...);
    char buffer[256];
    int len = snprintf(buffer, 256, message.c_str(), args...);
    std::string msg(buffer,len);
    log(SensorLoggerLevel::info, msg);
  }

  template <typename ... Types>
  void fault(const std::string& message, const Types&... args) {
    // std::string msg =  fmt::format(message,args...);
    char buffer[256];
    int len = snprintf(buffer, 256, message.c_str(), args...);
    std::string msg(buffer,len);
    log(SensorLoggerLevel::fault, msg);
  }

private:
  inline void log(SensorLoggerLevel lvl, std::string msg) {
    mSink->log(lvl, msg);
  }

  std::shared_ptr<SensorLoggingSink> mSink;
  std::string mSubsystem;
  std::string mCategory;
};

} // end namespace
} // end namespace

#endif