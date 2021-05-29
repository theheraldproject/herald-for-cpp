//  Copyright 2020-2021 Herald Project Contributors
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

  std::string description() const noexcept {
    return std::to_string((short)unit) + ":" + std::to_string(value);
  }

  operator std::string() const noexcept {
    return description();
  }
};


} // end namespace
} // end namespace

#endif