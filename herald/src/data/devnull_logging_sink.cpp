//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/data/sensor_logger.h"
#include "herald/data/devnull_logging_sink.h"

#include <iostream>

namespace herald::data {

DevNullLoggingSink::DevNullLoggingSink() = default;
DevNullLoggingSink::~DevNullLoggingSink() = default;

void
DevNullLoggingSink::log(const std::string& subsystem, const std::string& category, SensorLoggerLevel level, std::string message)
{
  ; // Literally do nothing... like cat-ing to /dev/null
}

}