//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PROXIMITY_H
#define PROXIMITY_H

#include <string>

namespace herald {
namespace datatype {

enum class ProximityMeasurementUnit : short {
  RSSI, RTT
};

struct Proximity {
  ProximityMeasurementUnit unit;
  double value;

  std::string description() {
    return std::to_string((short)unit) + ":" + std::to_string(value);
  }

  std::string toString() {
    return description();
  }
};


} // end namespace
} // end namespace

#endif