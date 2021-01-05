//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_H
#define SENSOR_H

#include "sensor_delegate.h"

namespace herald {

class Sensor {
public:
  Sensor() = default;
  virtual ~Sensor() = default;

  virtual void add(std::shared_ptr<SensorDelegate> delegate) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
};




} // end namespace

#endif