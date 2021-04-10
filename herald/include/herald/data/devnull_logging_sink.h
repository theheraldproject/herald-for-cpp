//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEVNULL_LOGGING_SINK
#define HERALD_DEVNULL_LOGGING_SINK

#include "herald/data/sensor_logger.h"

namespace herald::data {

struct DevNullLoggingSink {
  DevNullLoggingSink();
  ~DevNullLoggingSink();

  void log(const std::string& subsystem, const std::string& category, SensorLoggerLevel level, std::string message);
};

}

#endif