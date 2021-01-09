//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_coordinator.h"
#include "herald/engine/activities.h"

#include <memory>

namespace herald {
namespace ble {

using namespace herald::engine;

class HeraldProtocolBLECoordinationProvider::Impl {
public:
  Impl();
  ~Impl();

};

HeraldProtocolBLECoordinationProvider::Impl::Impl()
{
  ;
}

HeraldProtocolBLECoordinationProvider::Impl::~Impl()
{
  ;
}



HeraldProtocolBLECoordinationProvider::HeraldProtocolBLECoordinationProvider()
  : mImpl(std::make_unique<Impl>())
{
  ;
}


HeraldProtocolBLECoordinationProvider::~HeraldProtocolBLECoordinationProvider()
{
  ;
}


std::vector<FeatureTag>
HeraldProtocolBLECoordinationProvider::connectionsProvided()
{
  return std::vector<FeatureTag>();
}





std::vector<std::pair<FeatureTag,Priority>>
HeraldProtocolBLECoordinationProvider::requiredConnections()
{
  return std::vector<std::pair<FeatureTag,Priority>>();
}

std::vector<Activity>
HeraldProtocolBLECoordinationProvider::requiredActivities()
{
  return std::vector<Activity>();
}


}
}