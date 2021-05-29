//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_SENSOR_H
#define BLE_SENSOR_H

#include "../sensor.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// Tagging interface
class BLESensor : public Sensor {
public:
  BLESensor() = default;
  virtual ~BLESensor() = default;

  // Remaining methods inherited as pure virtual from Sensor class
};

} // end namespace
} // end namespace

#endif