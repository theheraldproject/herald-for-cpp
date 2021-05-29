//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_STATE_H
#define SENSOR_STATE_H

namespace herald {
namespace datatype {

enum class SensorState : short {
  /// Sensor is powered on, active and operational
  on,
  /// Sensor is powered off, inactive and not operational
  off,
  /// Sensor is not available
  unavailable
};

} // end namespace
} // end namespace

#endif