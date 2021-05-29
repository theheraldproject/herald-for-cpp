//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_WGS84_H
#define HERALD_WGS84_H

#include "location_reference.h"

#include <string>

namespace herald {
namespace datatype {

struct WGS84CircularAreaLocationReference : public LocationReference {
  double latitude;
  double longitude;
  double altitude;
  double radius;

  std::string description() override {
    return "WGS84(lat=" + std::to_string(latitude) + ",lon=" + std::to_string(longitude) + ",alt=" + std::to_string(altitude) + ",radius=" + std::to_string(radius) + ")";
  }

};



struct WGS84PointLocationReference : public LocationReference {
  double latitude;
  double longitude;
  double altitude;

  std::string description() override {
    return "WGS84(lat=" + std::to_string(latitude) + ",lon=" + std::to_string(longitude) + ",alt=" + std::to_string(altitude) + ")";
  }

};


} // end namespace
} // end namespace

#endif