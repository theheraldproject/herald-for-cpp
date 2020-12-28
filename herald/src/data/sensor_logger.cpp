//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "data/sensor_logger.h"

#include <string>

namespace herald {
namespace data {
  
std::string SensorLogger::sComma = ",";
std::string SensorLogger::sDebug = ",debug,";
std::string SensorLogger::sInfo  = ",info,";
std::string SensorLogger::sFault = ",fault,";

}
}