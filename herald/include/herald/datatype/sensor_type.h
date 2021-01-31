//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_TYPE_H
#define SENSOR_TYPE_H

#include <string>

namespace herald {
namespace datatype {

/// Sensor type as qualifier for target identifier.
enum class SensorType : short {
  /// Bluetooth Low Energy (BLE)
  BLE,
  /// GPS location sensor
  GPS,
  /// Physical beacon, e.g. iBeacon
  BEACON,
  /// Accelerometer
  ACCELEROMETER,
  /// Ultrasound audio beacon.
  ULTRASOUND,
  /// Future / other
  OTHER
};

std::string str(SensorType t) noexcept;

} // end namespace
} // end namespace

#endif