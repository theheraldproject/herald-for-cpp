//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef DEVICE_H
#define DEVICE_H

#include "datatype/target_identifier.h"
#include "datatype/time_interval.h"

#include <optional>

namespace herald {

using namespace herald::datatype;

class Device {
public:
  Device(TargetIdentifier identifier) : m_identifier(identifier) { };
  virtual ~Device() = default;

  virtual std::optional<TimeInterval> timeIntervalSinceLastUpdate() const = 0;
  virtual const TargetIdentifier& identifier() const { return m_identifier; };

private:
  TargetIdentifier m_identifier;
};

}

#endif
