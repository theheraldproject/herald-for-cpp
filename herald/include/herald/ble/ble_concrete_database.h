//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_DATABASE_H
#define HERALD_BLE_CONCRETE_DATABASE_H

#include "ble_database.h"
#include "ble_receiver.h"
#include "ble_sensor.h"
#include "ble_transmitter.h"
#include "ble_concrete.h"
#include "ble_protocols.h"
#include "bluetooth_state_manager.h"
#include "ble_device_delegate.h"
#include "filter/ble_advert_parser.h"
#include "../payload/payload_data_supplier.h"
#include "../context.h"
#include "../data/sensor_logger.h"
#include "ble_sensor_configuration.h"
#include "ble_coordinator.h"
#include "../datatype/bluetooth_state.h"

#include <memory>
#include <vector>
#include <array>
#include <algorithm>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;


/// \brief Provides a callable that assists in ordering for most recently updated BLEDevice
struct last_updated_descending {
  bool operator()(const BLEDevice& a, const BLEDevice& b) {
    return a.timeIntervalSinceLastUpdate() > b.timeIntervalSinceLastUpdate(); // opposite order
  }
};

template <typename ContextT, std::size_t MaxDevicesCached = 10>
class ConcreteBLEDatabase : public BLEDatabase, public BLEDeviceDelegate /*, public std::enable_shared_from_this<ConcreteBLEDatabase<ContextT>>*/  {
public:
  static constexpr std::size_t MaxDevices = MaxDevicesCached;

  ConcreteBLEDatabase(ContextT& context)
  : ctx(context),
    delegates(),
    devices()
    HLOGGERINIT(context,"herald","ConcreteBLEDatabase")
  {
    ;
  }

  ConcreteBLEDatabase(const ConcreteBLEDatabase& from) = delete;
  ConcreteBLEDatabase(ConcreteBLEDatabase&& from) = delete;

  ~ConcreteBLEDatabase() = default;

  // BLE Database overrides

  void add(BLEDatabaseDelegate& delegate) override {
    delegates.emplace_back(delegate);
  }

  // Creation overrides
  BLEDevice& device(const BLEMacAddress& mac, const Data& advert/*, const RSSI& rssi*/) override {
    // Check by MAC first
    TargetIdentifier targetIdentifier((Data)mac);
    auto results = matches([&targetIdentifier](const BLEDevice& d) {
      return d.identifier() == targetIdentifier;
    });
    if (results.size() != 0) {
      // HTDBG("DEVICE ALREADY KNOWN BY MAC");
      // Assume advert details are known already
      return results.front(); // TODO ensure we send back the latest, not just the first match
      // res->rssi(rssi);
      // return res;
    }

    // Now check by pseudo mac
    auto segments = BLEAdvertParser::extractSegments(advert,0);
    // HTDBG("segments:-");
    // HTDBG(std::to_string(segments.size()));
    auto manuData = BLEAdvertParser::extractManufacturerData(segments);
    auto heraldDataSegments = BLEAdvertParser::extractHeraldManufacturerData(manuData);
    // HTDBG("herald data segments:-");
    // HTDBG(std::to_string(heraldDataSegments.size()));
    // auto device = db->device(bleMacAddress); // For most devices this will suffice

    // TODO Check for public herald service in ADV_IND packet - shown if an Android device, wearable or beacon in zephyr
    // auto serviceData128 = BLEAdvertParser::extractServiceUUID128Data(segments);
    // bool hasHeraldService = false;
    // for (auto& service : serviceData128) {
    //   if (service.uuid == heraldUuidData) {
    //     hasHeraldService = true;
    //     HTDBG("FOUND DEVICE ADVERTISING HERALD SERVICE");
    //     device->operatingSystem(BLEDeviceOperatingSystem::android);
    //   }
    // }
    

    if (0 != heraldDataSegments.size()) {
      // HTDBG("Found Herald Android pseudo device address in advert");
      // Try to FIND by pseudo first
      BLEMacAddress pseudo(heraldDataSegments.front());
      auto samePseudo = matches([&pseudo](const BLEDevice& d) {
        return d.pseudoDeviceAddress() == pseudo;
      });
      if (0 != samePseudo.size()) {
        // HTDBG("FOUND EXISTING DEVICE BY PSEUDO");
        return samePseudo.front();
      }
      // HTDBG("CREATING NEW DEVICE BY MAC AND PSEUDO ONLY");
      // Now create new device with mac and pseudo
      auto& newDevice = device(mac,pseudo);
      assignAdvertData(newDevice,std::move(segments), manuData);
      // newDevice->rssi(rssi);
      return newDevice;
    }

    // HTDBG("CREATING NEW DEVICE BY MAC ONLY");

    // Now create a device just from a mac
    auto& newDevice = device(targetIdentifier);
    // HTDBG("Got new device");
    assignAdvertData(newDevice,std::move(segments), manuData);
    // newDevice->rssi(rssi);
    // HTDBG("Assigned advert data");
    return newDevice;
  }

  BLEDevice& device(const BLEMacAddress& mac, const BLEMacAddress& pseudo) override {
    auto samePseudo = matches([&pseudo](const BLEDevice& d) {
      return d.pseudoDeviceAddress() == pseudo;
    });
    if (0 == samePseudo.size()) {
      auto& ptr = device(TargetIdentifier((Data)pseudo));
      ptr.pseudoDeviceAddress(pseudo);
      return ptr;
    }
    // get most recent and clone, then attach
    auto comp = last_updated_descending();
    std::sort(samePseudo.begin(),samePseudo.end(), comp); // functional style
    BLEDevice& updatedDevice = samePseudo.front();
    // TODO support calling card
    // auto toShare = shareDataAcrossDevices(pseudo);
    // if (toShare.has_value()) {
    //   updatedDevice.payloadData(toShare);
    // }
    
    // Has pseudo address so must be android
    updatedDevice.operatingSystem(BLEDeviceOperatingSystem::android);

    // register new device discovery date
    updatedDevice.registerDiscovery(Date());

    // devices.push_back(updatedDevice);
    for (auto& delegate : delegates) {
      delegate.get().bleDatabaseDidCreate(updatedDevice); // may be new with a new service
    }
    return updatedDevice;
  }

  BLEDevice& device(const BLEMacAddress& mac) override {
    return device(TargetIdentifier((Data)mac));
  }

  BLEDevice& device(const PayloadData& payloadData) override {
    auto results = matches([&payloadData](const BLEDevice& d) {
      auto payload = d.payloadData();
      if (!payload.has_value()) {
        return false;
      }
      return (*payload)==payloadData;
    });
    if (results.size() != 0) {
      return results.front(); // TODO ensure we send back the latest, not just the first match
    }
    BLEDevice& newDevice = devices[indexAvailable()];
    newDevice.reset(TargetIdentifier(payloadData),*this);

    for (auto& delegate : delegates) {
      delegate.get().bleDatabaseDidCreate(newDevice);
    }
    newDevice.payloadData(payloadData); // has to be AFTER create called
    return newDevice;
  }

  BLEDevice& device(const TargetIdentifier& targetIdentifier) override {
    auto results = matches([&targetIdentifier](const BLEDevice& d) {
      return d.identifier() == targetIdentifier;
    });
    if (results.size() != 0) {
      return results.front(); // TODO ensure we send back the latest, not just the first match
    }
    HTDBG("New target identified: {}",(std::string)targetIdentifier);
    BLEDevice& newDevice = devices[indexAvailable()];
    newDevice.reset(targetIdentifier,*this);

    for (auto& delegate : delegates) {
      delegate.get().bleDatabaseDidCreate(newDevice);
    }
    return newDevice;
  }
  
  // Introspection overrides
  std::size_t size() const override {
    std::size_t count = 0;
    for (auto& d : devices) {
      if (d.state() != BLEDeviceState::uninitialised) {
        ++count;
      }
    }
    return count;
  }

  std::vector<std::reference_wrapper<BLEDevice>> matches(
    const std::function<bool(const BLEDevice&)>& matcher) override {
    std::vector<std::reference_wrapper<BLEDevice>> results;
    // in the absence of copy_if in C++20... Just copies the pointers not the objects
    for (auto iter = devices.begin();iter != devices.end();++iter) {
      if (BLEDeviceState::uninitialised != iter->state() && matcher(*iter)) {
        results.push_back(std::reference_wrapper<BLEDevice>(*iter));
      }
    }
    return results;
  }

  /// Cannot name a function delete in C++. remove is common.
  void remove(const TargetIdentifier& targetIdentifier) override {
    auto found = std::find_if(devices.begin(),devices.end(),
      [&targetIdentifier](BLEDevice& d) -> bool {
        return d.identifier() == targetIdentifier;
      }
    );
    if (found != devices.end()) {
      BLEDevice& toRemove = *found;
      remove(toRemove);
    }
  }

  // BLE Device Delegate overrides
  void device(const BLEDevice& device, BLEDeviceAttribute didUpdate) override {
    // Update any internal DB state as necessary (E.g. payload received and its a duplicate as mac has rotated)
    if (BLEDeviceAttribute::payloadData == didUpdate) {
      // check for all devices with this payload that are NOT THIS device
      auto oldMacsForSamePayload = matches([device](auto& devRef) {
        return devRef.identifier() != device.identifier() && 
               devRef.payloadData().has_value() && devRef.payloadData() == device.payloadData();
      });
      for (auto& oldMacDevice : oldMacsForSamePayload) {
        remove(oldMacDevice.get().identifier());
      }
    }

    // Now send update to delegates
    for (auto& delegate : delegates) {
      delegate.get().bleDatabaseDidUpdate(device, didUpdate); // TODO verify this is the right onward call
    }
  }

private:
  void assignAdvertData(BLEDevice& newDevice, std::vector<BLEAdvertSegment>&& toMove, 
    const std::vector<BLEAdvertManufacturerData>& manuData)
  {
    newDevice.advertData(std::move(toMove));

    // If it's an apple device, check to see if its on our ignore list
    auto appleDataSegments = BLEAdvertParser::extractAppleManufacturerSegments(manuData);
    if (0 != appleDataSegments.size()) {
      HTDBG("Found apple device");
      // HTDBG((std::string)mac);
      newDevice.operatingSystem(BLEDeviceOperatingSystem::ios);
      // TODO see if we should ignore this Apple device
      // TODO abstract these out eventually in to BLEDevice class
      bool ignore = false;
      /*
          "^10....04",
          "^10....14",
          "^0100000000000000000000000000000000",
          "^05","^07","^09",
          "^00","^1002","^06","^08","^03","^0C","^0D","^0F","^0E","^0B"
      */
      for (auto& segment : appleDataSegments) {
        HTDBG(segment.data.hexEncodedString());
        switch (segment.type) {
          case 0x00:
          case 0x05:
          case 0x07:
          case 0x09:
          case 0x06:
          case 0x08:
          case 0x03:
          case 0x0C:
          case 0x0D:
          case 0x0F:
          case 0x0E:
          case 0x0B:
            ignore = true;
            break;
          case 0x10:
            // check if second is 02
            if (segment.data.at(0) == std::byte(0x02)) {
              ignore = true;
            } else {
              // Check 3rd data bit for 14 or 04
              if (segment.data.at(2) == std::byte(0x04) || segment.data.at(2) == std::byte(0x14)) {
                ignore = true;
              }
            }
            break;
          default:
            break;
        }
      }
      if (ignore) {
        HTDBG(" - Ignoring Apple device due to Apple data filter");
        newDevice.ignore(true);
      } else {
        // Perform GATT service discovery to check for Herald service
        // NOTE: Happens from Connection request (handled by BLE Coordinator)
        HTDBG(" - Unknown apple device... Logging so we can discover services later");
      }
    } else {
      // Not a Herald android or any iOS - so inspect later (beacon or wearable)
      HTDBG("Unknown non Herald device - inspecting (might be a venue beacon or wearable)");
      // HTDBG((std::string)mac);
    }
  }

  void remove(BLEDevice& toRemove) {
    // Don't call delete/update if this device has never been initialised
    if (toRemove.state() == BLEDeviceState::uninitialised) {
      return;
    }
    for (auto& delegate : delegates) {
      delegate.get().bleDatabaseDidDelete(toRemove);
    }
    toRemove.state(BLEDeviceState::uninitialised);
    // TODO validate all other device data is reset
  }

  std::size_t indexAvailable() noexcept {
    for (std::size_t idx = 0;idx < devices.size();++idx) {
      auto& device = devices[idx];
      if (device.state().has_value() && BLEDeviceState::uninitialised == device.state().value()) {
        return idx;
      }
    }
    // If we've got here then there is no space available
    // Remove the oldest by lastUpdated
    auto comp = last_updated_descending();
    std::size_t oldestIndex = 0;
    for (std::size_t idx = 0;idx < devices.size();++idx) {
      if (!comp(devices[oldestIndex],devices[idx])) {
        // new oldest
        oldestIndex = idx;
      }
    }
    // Now notify we're deleting this device
    auto& oldest = devices[oldestIndex];
    remove(oldest);
    // Now re-use this reference
    return oldestIndex;
  }

  ContextT& ctx;
  std::vector<std::reference_wrapper<BLEDatabaseDelegate>> delegates;
  std::array<BLEDevice,MaxDevices> devices; // bool = in-use (not 'removed' from DB)

  HLOGGER(ContextT);
};

}
}

#endif