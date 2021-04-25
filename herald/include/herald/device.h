//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEVICE_H
#define HERALD_DEVICE_H

#include "datatype/target_identifier.h"
#include "datatype/time_interval.h"

#include <optional>

namespace herald {

using namespace herald::datatype;

///
/// \brief Generic abstraction of a particular local proximate device type.
/// 
/// Could be a Bluetooth Low Energy device (BLEDevice) or some other technology.
/// 
/// Only implemented in final version to allow TimeInterval and other
/// potentially platform specific implementation details to be overridden.
///
/// \sa herald::ble::BLEDevice
///
class Device {
public:
  Device() = default;
  virtual ~Device() = default;

  virtual Date created() const = 0;
  virtual TimeInterval timeIntervalSinceLastUpdate() const = 0;
  virtual const TargetIdentifier& identifier() const = 0;
  virtual void identifier(const TargetIdentifier& toCopyFrom) = 0;
};

}

#endif
