//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_COORDINATION_PROVIDER_H
#define BLE_COORDINATION_PROVIDER_H

#include "../context.h"
#include "../sensor.h"
#include "ble_database.h"
#include "ble_protocols.h"

#include <memory>
#include <functional>
#include <optional>
#include <tuple>

namespace herald {
namespace ble {

class HeraldProtocolBLECoordinationProvider : public CoordinationProvider {
public:
  HeraldProtocolBLECoordinationProvider(std::shared_ptr<Context> ctx, std::shared_ptr<BLEDatabase> db, std::shared_ptr<HeraldProtocolV1Provider> provider);
  ~HeraldProtocolBLECoordinationProvider();

  // Overrides
  
  /** What connections does this Sensor type provide for Coordination **/
  std::vector<FeatureTag> connectionsProvided() override;
  // void provision(const std::vector<PrioritisedPrerequisite>& requested,
  //   const ConnectionCallback& connCallback) override;
  std::vector<PrioritisedPrerequisite> provision(
    const std::vector<PrioritisedPrerequisite>& requested) override;

  // Runtime coordination callbacks
  /** Get a list of what connections are required to which devices now (may start, maintain, end (if not included)) **/
  std::vector<PrioritisedPrerequisite> requiredConnections() override;
  std::vector<Activity> requiredActivities() override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

}
}

#endif