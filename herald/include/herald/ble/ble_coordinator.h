//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_COORDINATION_PROVIDER_H
#define BLE_COORDINATION_PROVIDER_H

#include "../context.h"
#include "../sensor.h"

#include <memory>
#include <functional>
#include <optional>

namespace herald {
namespace ble {

class HeraldProtocolBLECoordinationProvider : public CoordinationProvider {
public:
  HeraldProtocolBLECoordinationProvider();
  ~HeraldProtocolBLECoordinationProvider();

  // Overrides
  
  /** What connections does this Sensor type provide for Coordination **/
  std::vector<FeatureTag> connectionsProvided() override;

  // Runtime coordination callbacks
  /** Get a list of what connections are required to which devices now (may start, maintain, end (if not included)) **/
  std::vector<std::pair<FeatureTag,Priority>> requiredConnections() override;
  std::vector<Activity> requiredActivities() override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

}
}

#endif