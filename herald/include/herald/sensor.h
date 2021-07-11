//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SENSOR_H
#define HERALD_SENSOR_H

#include "sensor_delegate.h"
#include "engine/activities.h"

namespace herald {

using namespace herald::engine;

/// \brief The base Sensor class that all Sensors implement.
///
/// A Sensor could be a bluetooth transmitter or receiver (scanner), or an NFC
/// receiver, or some other proximity sensor (E.g. UWB radio).
class Sensor {
public:
  Sensor() = default;
  virtual ~Sensor() = default;

  template <typename SensorDelegateT>
  void add(const SensorDelegateT& delegate) {
    ;
  }
  virtual void start() = 0;
  virtual void stop() = 0;

  /// \brief For complex sensor coordination support, if required - Since v1.2-beta3
  virtual std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() = 0;
};




} // end namespace

#endif