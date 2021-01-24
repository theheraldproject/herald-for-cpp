//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_coordinator.h"
#include "herald/engine/activities.h"
#include "herald/ble/ble_protocols.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/ble_sensor_configuration.h"

#include <memory>
#include <future>

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

  std::vector<PrioritisedPrerequisite> previouslyProvisioned;

  // int currentConnections;
  // int maxConnections;

  HLOGGER;
};

HeraldProtocolBLECoordinationProvider::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BLEDatabase> bledb,std::shared_ptr<HeraldProtocolV1Provider> provider)
  : context(ctx),
    db(bledb),
    pp(provider),
    previouslyProvisioned()
    // ,
    // currentConnections(0),
    // maxConnections(20)
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

// void
// HeraldProtocolBLECoordinationProvider::provision(const std::vector<PrioritisedPrerequisite>& requested,
//   const ConnectionCallback& connCallback)
std::vector<PrioritisedPrerequisite>
HeraldProtocolBLECoordinationProvider::provision(
  const std::vector<PrioritisedPrerequisite>& requested)
{
  if (requested.empty()) {
    HDBG("No connections requested for provisioning");
  } else {
    HDBG("Provisioning connections");
  }

  // Remove those previously provisoned that we no longer require
  for (auto& previous : mImpl->previouslyProvisioned) {
    bool found = false;
    for (auto& req : requested) {
      found = found || std::get<2>(req)==std::get<2>(previous); // comparing target values
    }
    if (!found) {
      HDBG(" - Found connection to remove");
      auto& optTarget = std::get<2>(previous);
      // We can't disconnect from 'all', so check for target
      if (optTarget.has_value()) {
        // Ignoring closed true/false result
        mImpl->pp->closeConnection(optTarget.value());
      }
    }
  }

  // Now provision new connections
  std::vector<PrioritisedPrerequisite> provisioned;
  // For this provider, a prerequisite is a connection to a remote target identifier over Bluetooth
  auto requestIter = requested.cbegin();
  bool lastConnectionSuccessful = true;
  while (/*mImpl->currentConnections < mImpl->maxConnections && */
         lastConnectionSuccessful && 
         requestIter != requested.cend()) {
    HDBG(" - Satisfying prereq");
    // HDBG(" - currentConnections currently:-");
    // HDBG(std::to_string(mImpl->currentConnections));
    auto& req = *requestIter;
    // See if we're already connected
    // If so, add to provisioned list
    // If not, try to connect
    auto& optTarget = std::get<2>(req);
    if (optTarget.has_value()) {
      HDBG(" - Have defined target for this prerequisite. Requesting connection be made available.");
      // std::future<void> fut = std::async(std::launch::async,
      //     &HeraldProtocolV1Provider::openConnection,mImpl->pp,
      //     optTarget.value(),[&lastConnectionSuccessful] (
      //     const TargetIdentifier& targetForConnection, bool success) -> void {
      //   lastConnectionSuccessful = success;
      // });
      // fut.get(); // TODO FIND OUT HOW TO DO THIS FUTURE WAITING FUNCTIONALITY

      lastConnectionSuccessful = mImpl->pp->openConnection(optTarget.value());

      // If successful, add to provisioned list
      if (lastConnectionSuccessful) {
        HDBG(" - Ensuring connection successful");
        provisioned.push_back(req);
        // mImpl->currentConnections++;
      } else {
        HDBG(" - Ensuring connection UNSUCCESSFUL");
      }
    } else {
      HDBG(" - No defined target, returning satisfied - always true for Herald BLE");
      // if target not specified then it just means any open connection, so return OK
      provisioned.push_back(req);
      // TODO determine what to do here if sensor stopped, bluetooth disabled, or no connections open
    }
    // move forward in iterator
    requestIter++;

    lastConnectionSuccessful = true;
  }

  mImpl->previouslyProvisioned = provisioned;

  // TODO schedule disconnection from not required items (E.g. after minimum connection time)
  //  - we already do this if provision is called, but not after a time period
  //  - Bluetooth timeout will deal with the underlying connection, but not any intermediate state in the PP instance

  // HDBG("Returning from provision");
  // connCallback(provisioned);
  return provisioned;
}



std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>>
HeraldProtocolBLECoordinationProvider::requiredConnections()
{
  std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>> results;

  // Add all targets in database that are not known
  // auto newConns = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
  //   return !device->ignore() &&
  //     (
  //       device->operatingSystem() == BLEDeviceOperatingSystem::unknown // not yet determined OS
  //       ||
  //       !device->payloadData().has_value() // Know the OS, but not the payload (ID)
  //       ||
  //       device->immediateSendData().has_value()
  //     )
  //     ;
  // });
  auto newConns = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return !device->ignore() &&
      (
        !device->hasService(BLESensorConfiguration::serviceUUID)
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

  // DEBUG ONLY ELEMENTS
  if (newConns.size() > 0) {
    // print debug info about the BLE Database
    HDBG("BLE DATABASE CURRENT CONTENTS:-");
    auto allDevices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
      return true;
    });
    for (auto& device : allDevices) {
      std::string di(" - ");
      BLEMacAddress mac((Data)device->identifier());
      di += (std::string)mac;
      di += ", os=";
      auto os = device->operatingSystem();
      if (os.has_value()) {
        if (herald::ble::BLEDeviceOperatingSystem::ios == os) {
          di += "ios";
        } else if (herald::ble::BLEDeviceOperatingSystem::android == os) {
          di += "android";
        }
      } else {
        di += "unknown";
      }
      di += ", ignore=";
      auto ignore = device->ignore();
      if (ignore) {
        di += "true";
      } else {
        di += "false";
      }
      di += ", hasServices=";
      di += (device->hasServicesSet() ? "true" : "false");
      di += ", hasReadPayload=";
      di += (device->payloadData().has_value() ? device->payloadData().value().hexEncodedString() : "false");
      HDBG(di);
    }
  } else {
    // restart scanning when no connection activity is expected
    mImpl->pp->restartScanningAndAdvertising();
  }

  return results;
}

std::vector<Activity>
HeraldProtocolBLECoordinationProvider::requiredActivities()
{
  std::vector<Activity> results;

  // State 0 - New device -> Read full advert data to see if DCT/Herald -> State Z, 1 or State 3
  // State 1 - Discover services for DCT/Herald on this device -> State Z, or 2
  // State 2 - New Herald BLE device -> Read payload -> State 3
  // State 3 - Steady state - do nothing
  // State 4 - Has immediateSend data -> Send immediate -> State 3
  // TODO check for nearby payloads
  // State 3 - Not seen in a while -> State X
  // State X - Out of range. No actions.
  // State Z - Ignore (not a relevant device for Herald). No actions.

  
  // auto state0Devices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
  //   return !device->ignore() && !device->pseudoDeviceAddress().has_value();
  // });
  auto state1Devices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return !device->ignore() && 
           !device->hasService(BLESensorConfiguration::serviceUUID);
  });
  auto state2Devices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return !device->ignore() && 
            device->hasService(BLESensorConfiguration::serviceUUID) &&
           !device->payloadData().has_value(); // TODO check for Herald transferred payload data (not legacy)
  });
  auto state4Devices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
    return !device->ignore() && 
            device->hasService(BLESensorConfiguration::serviceUUID) &&
            device->immediateSendData().has_value();
  });
  // TODO State X (timed out / out of range) devices filter check -> Then remove from BLEDatabase

  // auto relevantDevices = mImpl->db->matches([](std::shared_ptr<BLEDevice> device) -> bool {
  //   return device->operatingSystem() == BLEDeviceOperatingSystem::unknown ||
  //     (
  //       device->ignore() == false && (
  //         !device->payloadData().has_value()
  //         ||
  //         device->immediateSendData().has_value()
  //       )
  //     )
  //   ;
  // });
  
  // NOTE State 0 is handled by the Herald BLE scan function, and so has no specific activity

  // State 1 - discovery Herald service
  for (auto device : state1Devices) {
    results.emplace_back(Activity{
      .priority = Priorities::High + 10,
      .name = "herald-service-discovery",
      .prerequisites = std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
        1,
        std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
          herald::engine::Features::HeraldBluetoothProtocolConnection,
          device->identifier()
        }
      },
      // .executor = [this](const Activity activity, CompletionCallback callback) -> void {
      //   // fill this out
      //   mImpl->pp->serviceDiscovery(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
        mImpl->pp->serviceDiscovery(activity);
        return {};
      }
    });
  }

  // State 2 - read herald payload(s)
  for (auto device : state2Devices) {
    results.emplace_back(Activity{
      .priority = Priorities::High + 9,
      .name = "herald-read-payload",
      .prerequisites =  std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
        1,
        std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
          herald::engine::Features::HeraldBluetoothProtocolConnection,
          device->identifier()
        }
      },
      // .executor = [this](const Activity activity, CompletionCallback callback) -> void {
      //   // fill this out
      //   mImpl->pp->readPayload(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
       mImpl->pp->readPayload(activity);
       return {};
      }
    });
  }
  // TODO add check for sensor config payload timeout in above IF
  // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsModelCharacteristic() && device.model() == null
  // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsDeviceNameCharacteristic() && device.deviceName() == null
  
  // State 4 - Has data for immediate send
  for (auto device : state4Devices) {
    results.emplace_back(Activity{
      .priority = Priorities::Default + 10,
      .name = "herald-immediate-send-targeted",
      .prerequisites =  std::vector<std::tuple<FeatureTag,std::optional<TargetIdentifier>>>{
        1,
        std::tuple<FeatureTag,std::optional<TargetIdentifier>>{
          herald::engine::Features::HeraldBluetoothProtocolConnection,
          device->identifier()
        }
      },
      // For std::async based platforms:-
      // .executor = [this](const Activity activity, CompletionCallback callback) -> void {
      //   // fill this out
      //   mImpl->pp->immediateSend(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
        mImpl->pp->immediateSend(activity);
        return {};
      }
    });
    // TODO add immediate send all support
    // TODO add read of nearby payload data from remotes
  }
  return results;
}


}
}