//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEVICE_H
#define HERALD_DEVICE_H

#include "datatype/target_identifier.h"
#include "datatype/time_interval.h"

#include <optional>

namespace herald {

using namespace herald::datatype;

/**
 * Only implemented in final version to allow TimeInterval and other
 * potentially platform specific implementation details to be overridden
 */
class Device {
public:
  Device() = default;
  virtual ~Device() = default;

  virtual std::optional<TimeInterval> timeIntervalSinceLastUpdate() const = 0;
  virtual const TargetIdentifier& identifier() const = 0;
};

}

#endif
