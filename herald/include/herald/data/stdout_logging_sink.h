//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_STDOUT_LOGGING_SINK
#define HERALD_STDOUT_LOGGING_SINK

#include "herald/data/sensor_logger.h"

namespace herald::data {

struct StdOutLoggingSink {
  StdOutLoggingSink();
  ~StdOutLoggingSink();

  void log(const std::string& subsystem, const std::string& category, SensorLoggerLevel level, std::string message);
};

}

#endif