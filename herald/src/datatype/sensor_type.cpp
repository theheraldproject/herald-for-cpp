//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/sensor_type.h"

#include <string>

namespace herald::datatype {


std::string str(SensorType t) noexcept
{
  switch (t) {
    case SensorType::BLE:
      return "ble";
    case SensorType::GPS:
      return "gps";
    case SensorType::BEACON:
      return "beacon";
    case SensorType::ACCELEROMETER:
      return "accelerometer";
    case SensorType::ULTRASOUND:
      return "ultrasound";
    default:
      return "other";
  }
}

}