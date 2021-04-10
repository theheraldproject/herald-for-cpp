//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_COORDINATION_PROVIDER_H
#define BLE_COORDINATION_PROVIDER_H

#include "../context.h"
#include "../sensor.h"
#include "ble_database.h"
#include "ble_protocols.h"
#include "ble_coordinator.h"
#include "../engine/activities.h"
#include "ble_protocols.h"
#include "../data/sensor_logger.h"
#include "ble_sensor_configuration.h"

#include <memory>
#include <functional>
#include <optional>
#include <tuple>

namespace herald {
namespace ble {

template <typename ContextT, typename BLEDBT, typename ProviderT>
class HeraldProtocolBLECoordinationProvider : public CoordinationProvider {
public:
  HeraldProtocolBLECoordinationProvider(ContextT& ctx, BLEDBT& bledb, 
    ProviderT& provider) 
  : context(ctx),
    db(bledb),
    pp(provider),
    previouslyProvisioned(),
    iterationsSinceBreak(0),
    breakEvery(10),
    breakFor(10)
    HLOGGERINIT(ctx,"heraldble","coordinationprovider")
  {}

  ~HeraldProtocolBLECoordinationProvider() = default;

  // Overrides
  
  /** What connections does this Sensor type provide for Coordination **/
  std::vector<FeatureTag> connectionsProvided() override {
    return std::vector<FeatureTag>(1,herald::engine::Features::HeraldBluetoothProtocolConnection);
  }

  // void provision(const std::vector<PrioritisedPrerequisite>& requested,
  //   const ConnectionCallback& connCallback) override;
  std::vector<PrioritisedPrerequisite> provision(
    const std::vector<PrioritisedPrerequisite>& requested) override {
    if (requested.empty()) {
      // HTDBG("No connections requested for provisioning");
    } else {
      HTDBG("Provisioning connections");
    }

    // Remove those previously provisoned that we no longer require
    for (auto& previous : previouslyProvisioned) {
      bool found = false;
      for (auto& req : requested) {
        found = found || std::get<2>(req)==std::get<2>(previous); // comparing target values
      }
      if (!found) {
        HTDBG(" - Found connection to remove");
        auto& optTarget = std::get<2>(previous);
        // We can't disconnect from 'all', so check for target
        if (optTarget.has_value()) {
          // Ignoring closed true/false result
          pp.closeConnection(optTarget.value());
        }
      }
    }
  // Now provision new connections
  std::vector<PrioritisedPrerequisite> provisioned;
  // For this provider, a prerequisite is a connection to a remote target identifier over Bluetooth
  auto requestIter = requested.cbegin();
  bool lastConnectionSuccessful = true;
  while (/*currentConnections < maxConnections && */
         lastConnectionSuccessful && 
         requestIter != requested.cend()) {
    HTDBG(" - Satisfying prereq");
    // HTDBG(" - currentConnections currently:-");
    // HTDBG(std::to_string(currentConnections));
    auto& req = *requestIter;
    // See if we're already connected
    // If so, add to provisioned list
    // If not, try to connect
    auto& optTarget = std::get<2>(req);
    if (optTarget.has_value()) {
      HTDBG(" - Have defined target for this prerequisite. Requesting connection be made available.");
      // std::future<void> fut = std::async(std::launch::async,
      //     &HeraldProtocolV1Provider::openConnection,pp,
      //     optTarget.value(),[&lastConnectionSuccessful] (
      //     const TargetIdentifier& targetForConnection, bool success) -> void {
      //   lastConnectionSuccessful = success;
      // });
      // fut.get(); // TODO FIND OUT HOW TO DO THIS FUTURE WAITING FUNCTIONALITY

      lastConnectionSuccessful = pp.openConnection(optTarget.value());

      // If successful, add to provisioned list
      if (lastConnectionSuccessful) {
        HTDBG(" - Ensuring connection successful");
        provisioned.push_back(req);
        // currentConnections++;
      } else {
        HTDBG(" - Ensuring connection UNSUCCESSFUL");
      }
    } else {
      HTDBG(" - No defined target, returning satisfied - always true for Herald BLE");
      // if target not specified then it just means any open connection, so return OK
      provisioned.push_back(req);
      // TODO determine what to do here if sensor stopped, bluetooth disabled, or no connections open
    }
    // move forward in iterator
    requestIter++;

    lastConnectionSuccessful = true;
  }

  previouslyProvisioned = provisioned;

  // TODO schedule disconnection from not required items (E.g. after minimum connection time)
  //  - we already do this if provision is called, but not after a time period
  //  - Bluetooth timeout will deal with the underlying connection, but not any intermediate state in the PP instance

  // HTDBG("Returning from provision");
  // connCallback(provisioned);
  return provisioned;
  }

  // Runtime coordination callbacks
  /** Get a list of what connections are required to which devices now (may start, maintain, end (if not included)) **/
  std::vector<PrioritisedPrerequisite> requiredConnections() override {
    std::vector<std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>> results;

    // This ensures we break from making connections to allow advertising and scanning
    iterationsSinceBreak++;
    if (iterationsSinceBreak >= breakEvery &&
      iterationsSinceBreak < (breakEvery + breakFor) ) {
      HTDBG("###### Skipping connections - giving advertising & scanning a chance");
      // if (iterationsSinceBreak == breakEvery) { // incase it fails
        pp.restartScanningAndAdvertising();
      // }
      return results;
    } else if (iterationsSinceBreak == (breakEvery + breakFor) ) {
      // reset
      iterationsSinceBreak = 0;
    }

    // Remove expired devices
    auto expired = db.matches([/*this*/] (const std::shared_ptr<BLEDevice>& device) -> bool {
      auto interval = device->timeIntervalSinceLastUpdate();
      bool notZero = interval != TimeInterval::zero();
      bool isOld = interval > TimeInterval::minutes(15);
      // HTDBG("ID, created, Now, interval, notZero, isOld:-");
      // HTDBG((std::string)device->identifier());
      // HTDBG(std::to_string((long)device->created()));
      // HTDBG(std::to_string((long)Date()));
      // HTDBG((std::string)interval);
      // HTDBG(notZero?"true":"false");
      // HTDBG(isOld?"true":"false");
      return notZero && isOld;
    });
    for (auto& exp : expired) {
      db.remove(exp->identifier());
      HTDBG("Removing expired device with ID: ");
      HTDBG((std::string)exp->identifier());
      HTDBG("time since last update:-");
      HTDBG(std::to_string(exp->timeIntervalSinceLastUpdate()));
    }

    // Allow updates from ignored (for a time) status, to retry status
    auto tempIgnoredOS = db.matches([](const std::shared_ptr<BLEDevice>& device) -> bool {
      return device->operatingSystem() == BLEDeviceOperatingSystem::ignore;
    });
    for (auto& device : tempIgnoredOS) {
      // don't bother with separate activity right now - no connection required
      device->operatingSystem(BLEDeviceOperatingSystem::unknown);
    }


    // Add all targets in database that are not known
    auto newConns = db.matches([this](const std::shared_ptr<BLEDevice>& device) -> bool {
      return !device->ignore() &&
        (
          !device->hasService(context.getSensorConfiguration().serviceUUID)
          ||
          !device->payloadData().has_value() // Know the OS, but not the payload (ID)
          ||
          device->immediateSendData().has_value()
        )
        ;
    });
    for (auto& device : newConns) {
      results.emplace_back(herald::engine::Features::HeraldBluetoothProtocolConnection,
        herald::engine::Priorities::High,
        device->identifier()
      );
    }

    // TODO any other devices we may have outstanding work for that requires connections

    // DEBUG ONLY ELEMENTS
    if (newConns.size() > 0) {
      // print debug info about the BLE Database
      HTDBG("BLE DATABASE CURRENT CONTENTS:-");
      auto allDevices = db.matches([](const std::shared_ptr<BLEDevice>& device) -> bool {
        return true;
      });
      for (auto& device : allDevices) {
        std::string di(" - ");
        BLEMacAddress mac((Data)device->identifier());
        di += (std::string)mac;
        di += ", created=";
        di += std::to_string(device->created());
        di += ", pseudoAddress=";
        auto pseudo = device->pseudoDeviceAddress();
        if (pseudo.has_value()) {
          di += (std::string)pseudo.value();
        } else {
          di += "unset";
        }
        di += ", os=";
        auto os = device->operatingSystem();
        if (os.has_value()) {
          if (herald::ble::BLEDeviceOperatingSystem::ios == os) {
            di += "ios";
          } else if (herald::ble::BLEDeviceOperatingSystem::android == os) {
            di += "android";
          } else if (herald::ble::BLEDeviceOperatingSystem::unknown == os) {
            di += "unknown";
          } else if (herald::ble::BLEDeviceOperatingSystem::ignore == os) {
            di += "ignore";
          } else if (herald::ble::BLEDeviceOperatingSystem::android_tbc == os) {
            di += "android_tbc";
          } else if (herald::ble::BLEDeviceOperatingSystem::ios_tbc == os) {
            di += "ios_tbc";
          } else if (herald::ble::BLEDeviceOperatingSystem::shared == os) {
            di += "shared";
          }
        } else {
          di += "unknown/unset";
        }
        di += ", ignore=";
        auto ignore = device->ignore();
        if (ignore) {
          di += "true (for ";
          di += std::to_string(device->timeIntervalUntilIgnoreExpired().millis());
          di += " more secs)";
        } else {
          di += "false";
        }
        di += ", hasServices=";
        di += (device->hasServicesSet() ? "true" : "false");
        di += ", hasReadPayload=";
        di += (device->payloadData().has_value() ? device->payloadData().value().hexEncodedString() : "false");
        HTDBG(di);
      }
    } else {
      // restart scanning when no connection activity is expected
      pp.restartScanningAndAdvertising();
    }

    return results;
  }
  std::vector<Activity> requiredActivities() override {
  std::vector<Activity> results;

  // State 0 - New device -> Read full advert data to see if DCT/Herald -> State Z, 1 or State 3
  // State 1 - Discover services for DCT/Herald on this device -> State Z, or 2
  // State 2 - New Herald BLE device -> Read payload -> State 3
  // State 3 - Steady state - do nothing
  // State 4 - Has immediateSend data -> Send immediate -> State 3
  // TODO check for nearby payloads
  // State 3 - Not seen in a while -> State X
  // State X - Out of range. No actions.
  // State Z - Ignore (not a relevant device for Herald... right now). Ignore for a period of time. No actions.

  // TODO is IOS and needs payload sharing

  
  // auto state0Devices = db.matches([](std::shared_ptr<BLEDevice> device) -> bool {
  //   return !device->ignore() && !device->pseudoDeviceAddress().has_value();
  // });
  auto state1Devices = db.matches([this](const std::shared_ptr<BLEDevice>& device) -> bool {
    return !device->ignore() && 
           !device->receiveOnly() &&
           !device->hasService(context.getSensorConfiguration().serviceUUID);
  });
  auto state2Devices = db.matches([this](const std::shared_ptr<BLEDevice>& device) -> bool {
    return !device->ignore() && 
           !device->receiveOnly() &&
            device->hasService(context.getSensorConfiguration().serviceUUID) &&
           !device->payloadData().has_value(); // TODO check for Herald transferred payload data (not legacy)
  });
  auto state4Devices = db.matches([this](const std::shared_ptr<BLEDevice>& device) -> bool {
    return !device->ignore() && 
           !device->receiveOnly() &&
            device->hasService(context.getSensorConfiguration().serviceUUID) &&
            device->immediateSendData().has_value();
  });
  // TODO State X (timed out / out of range) devices filter check -> Then remove from BLEDatabase
  
  // NOTE State 0 is handled by the Herald BLE scan function, and so has no specific activity

  // State 1 - discovery Herald service
  for (auto& device : state1Devices) {
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
      //   pp->serviceDiscovery(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
        pp.serviceDiscovery(activity);
        return {};
      }
    });
  }

  // State 2 - read herald payload(s)
  for (auto& device : state2Devices) {
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
      //   pp->readPayload(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
       pp.readPayload(activity);
       return {};
      }
    });
  }
  // TODO add check for sensor config payload timeout in above IF
  // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsModelCharacteristic() && device.model() == null
  // TODO add BLESensorConfiguration.deviceIntrospectionEnabled && device.supportsDeviceNameCharacteristic() && device.deviceName() == null
  
  // State 4 - Has data for immediate send
  for (auto& device : state4Devices) {
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
      //   pp->immediateSend(activity,callback);
      // }
      .executor = [this](const Activity activity) -> std::optional<Activity> {
        // fill this out
        pp.immediateSend(activity);
        return {};
      }
    });
    // TODO add immediate send all support
    // TODO add read of nearby payload data from remotes
  }
  return results;
}

private:
  ContextT& context; 
  BLEDBT& db;
  ProviderT& pp;

  std::vector<PrioritisedPrerequisite> previouslyProvisioned;

  int iterationsSinceBreak;
  int breakEvery;
  int breakFor;

  HLOGGER(ContextT);
};

}
}

#endif