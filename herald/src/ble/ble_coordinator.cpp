//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_coordinator.h"
#include "herald/engine/activities.h"
#include "herald/ble/ble_protocols.h"
#include "herald/data/sensor_logger.h"

#include <memory>

namespace herald {
namespace ble {

using namespace herald::engine;

class HeraldProtocolBLECoordinationProvider::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BLEDatabase> bledb,std::shared_ptr<HeraldProtocolV1Provider> provider);
  ~Impl();

  std::shared_ptr<Context> context; 
  std::shared_ptr<BLEDatabase> db;
  std::shared_ptr<HeraldProtocolV1Provider> pp;

  HLOGGER;
};

HeraldProtocolBLECoordinationProvider::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BLEDatabase> bledb,std::shared_ptr<HeraldProtocolV1Provider> provider)
  : context(ctx),
    db(bledb),
    pp(provider)
    HLOGGERINIT(ctx,"heraldble","coordinationprovider")
{
  ;
}

HeraldProtocolBLECoordinationProvider::Impl::~Impl()
{
  ;
}



HeraldProtocolBLECoordinationProvider::HeraldProtocolBLECoordinationProvider(std::shared_ptr<Context> ctx, 
  std::shared_ptr<BLEDatabase> db, std::shared_ptr<HeraldProtocolV1Provider> provider)
  : mImpl(std::make_unique<Impl>(ctx,db,provider))
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

std::vector<PrioritisedPrerequisite>
HeraldProtocolBLECoordinationProvider::provision(std::vector<PrioritisedPrerequisite> requested)
{
  HDBG("Entered provision");
  std::vector<PrioritisedPrerequisite> provisioned;
  // For this provider, a prerequisite is a connection to a remote target identifier over Bluetooth
  auto requestIter = requested.cbegin();
  bool lastConnectionSuccessful = true;
  while (lastConnectionSuccessful && requestIter != requested.cend()) {
    HDBG(" - Satisfying prereq");
    auto& req = *requestIter;
    // See if we're already connected
    // If so, add to provisioned list
    // If not, try to connect
    auto& optTarget = std::get<2>(req);
    if (optTarget.has_value()) {
      HDBG(" - Have defined target");
      lastConnectionSuccessful = mImpl->pp->openConnection(optTarget.value());
      // If successful, add to provisioned list
      if (lastConnectionSuccessful) {
        HDBG(" - Opening new connection successful");
        provisioned.push_back(req);
      } else {
        HDBG(" - Opening new connection UNSUCCESSFUL");
      }
    } else {
      HDBG(" - No defined target, returning satisfied - always true for Herald BLE");
      // if target not specified then it just means any open connection, so return OK
      provisioned.push_back(req);
      // TODO determine what to do here if sensor stopped, bluetooth disabled, or no connections open
    }
    // move forward in iterator
    requestIter++;
    // TODO change the below to check for upper connection limit being reached
    lastConnectionSuccessful = true;
  }
  // TODO prioritise disconnection from not required items (E.g. after minimum connection time)

  HDBG("Returning from provision");
  return provisioned;
}



std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>>
HeraldProtocolBLECoordinationProvider::requiredConnections()
{
  std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>> results;

  // Add all targets in database that are not known
  auto newConns = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return !device->ignore() &&
      (
        device->operatingSystem() == BLEDeviceOperatingSystem::unknown // not yet determined OS
        ||
        !device->payloadData().has_value() // Know the OS, but not the payload (ID)
        ||
        device->immediateSendData().has_value()
      )
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
        .executor = [this](const Activity activity, CompletionCallback callback) -> void {
          // fill this out
          mImpl->pp->identifyOS(activity,callback);
          // call callback
          //callback(activity, {});
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
        .executor = [this](const Activity activity, CompletionCallback callback) -> void {
          // fill this out
          mImpl->pp->readPayload(activity,callback);
          // call callback
          //callback(activity, {});
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
        .executor = [this](const Activity activity, CompletionCallback callback) -> void {
          // fill this out
          mImpl->pp->immediateSend(activity,callback);
          // call callback
          //callback(activity, {});
        }
      });
    }
    // TODO add immediate send all support
    // TODO add read of nearby payload data from remotes
  }
  return results;
}


}
}