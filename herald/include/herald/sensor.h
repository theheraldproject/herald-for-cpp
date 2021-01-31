//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SENSOR_H
#define SENSOR_H

#include "sensor_delegate.h"
#include "engine/activities.h"

namespace herald {

using namespace herald::engine;

class Sensor {
public:
  Sensor() = default;
  virtual ~Sensor() = default;

  virtual void add(const std::shared_ptr<SensorDelegate>& delegate) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;

  /** For complex sensor coordination support, if required - Since v1.2-beta3 **/
  virtual std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() = 0;
};




} // end namespace

#endif