//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ZEPHYR_LOGGING_SINK_H
#define HERALD_ZEPHYR_LOGGING_SINK_H

#include "../sensor_logger.h"

// NOTE: Link Herald to the Zephyr logging system
// Set HERALD_LOG_LEVEL=4 for debug in CMake using add_definitions(-DHERALD_LOG_LEVEL=4 )
//   Defaults to 0 (OFF) - see herald/data/zephyr/zephyr_logging_sink.h
// #include <logging/log.h>

namespace herald {

namespace data {

class ZephyrLoggingSink {
public:
  ZephyrLoggingSink() = default;
  ~ZephyrLoggingSink() = default;

  void log(const std::string& subsystem,const std::string& category,
    SensorLoggerLevel level, const std::string& message);
};

} // end namespace
} // end namespace

#endif