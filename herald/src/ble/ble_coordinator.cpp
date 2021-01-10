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
  Impl(std::shared_ptr<BLEDatabase> bledb);
  ~Impl();

  std::shared_ptr<BLEDatabase> db;
};

HeraldProtocolBLECoordinationProvider::Impl::Impl(std::shared_ptr<BLEDatabase> bledb)
  : db(bledb)
{
  ;
}

HeraldProtocolBLECoordinationProvider::Impl::~Impl()
{
  ;
}



HeraldProtocolBLECoordinationProvider::HeraldProtocolBLECoordinationProvider(std::shared_ptr<BLEDatabase> db)
  : mImpl(std::make_unique<Impl>(db))
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
  return std::vector<FeatureTag>(1,herald::engine::Features::HeraldBluetoothProtocolConnection);
}





std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>>
HeraldProtocolBLECoordinationProvider::requiredConnections()
{
  std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>> results;

  // Add all targets in database that are not known
  auto newConns = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return device->operatingSystem() == BLEDeviceOperatingSystem::unknown // not yet determined OS
      ||
      !device->payloadData().has_value() // Know the OS, but not the payload (ID)
      ||
      device->immediateSendData().has_value()
      ;
  });
  for (auto device : newConns) {
    results.emplace_back(herald::engine::Features::HeraldBluetoothProtocolConnection,
      herald::engine::Priorities::High,
      device->identifier()
    );
  }

  // TODO any other devices we may have outstanding work for that requires connections

  return results;
}

std::vector<Activity>
HeraldProtocolBLECoordinationProvider::requiredActivities()
{
  std::vector<Activity> results;

  auto relevantDevices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return device->operatingSystem() == BLEDeviceOperatingSystem::unknown ||
      (
        device->ignore() == false && (
          !device->payloadData().has_value()
          ||
          device->immediateSendData().has_value()
        )
      )
    ;
  });
  for (auto device : relevantDevices) {
    if (device->operatingSystem() == BLEDeviceOperatingSystem::unknown) {
      results.emplace_back(Activity{
        .priority = Priorities::High + 10,
        .name = "determine-os",
        .prerequisites = std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
          1,
          std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
            herald::engine::Features::HeraldBluetoothProtocolConnection,
            device->identifier()
          }
        },
        .executor = [](const Activity activity, CompletionCallback callback) -> void {
          // TODO fill this out
          // call callback
          callback(activity, {});
        }
      });
    }
    if (!device->payloadData().has_value()) {
      results.emplace_back(Activity{
        .priority = Priorities::High + 9,
        .name = "read-payload",
        .prerequisites =  std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
          1,
          std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
            herald::engine::Features::HeraldBluetoothProtocolConnection,
            device->identifier()
          }
        },
        .executor = [](const Activity activity, CompletionCallback callback) -> void {
          // TODO fill this out
          // call callback
          callback(activity, {});
        }
      });
    }
    // TODO add check for sensor config payload timeout in above IF
    // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsModelCharacteristic() && device.model() == null
    // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsDeviceNameCharacteristic() && device.deviceName() == null
    if (device->immediateSendData().has_value()) {
      results.emplace_back(Activity{
        .priority = Priorities::Default + 10,
        .name = "immediate-send-targeted",
        .prerequisites =  std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
          1,
          std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
            herald::engine::Features::HeraldBluetoothProtocolConnection,
            device->identifier()
          }
        },
        .executor = [](const Activity activity, CompletionCallback callback) -> void {
          // TODO fill this out
          // call callback
          callback(activity, {});
        }
      });
    }
    // TODO add read of nearby payload data from remotes
  }
  return results;
}


}
}