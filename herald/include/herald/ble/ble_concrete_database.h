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

#include <array>
#include <algorithm>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;


/// \brief Provides a callable that assists in ordering for most recently updated BLEDevice
struct last_updated_descending {
  bool operator()(const BLEDevice& a, const BLEDevice& b) noexcept {
    return a.timeIntervalSinceLastUpdate() > b.timeIntervalSinceLastUpdate(); // opposite order
  }
  bool operator()(const std::optional<std::reference_wrapper<BLEDevice>>& lhs, const std::optional<std::reference_wrapper<BLEDevice>>& rhs) noexcept {
    if (lhs.has_value() && !rhs.has_value()) {
      return 1;
    }
    if (rhs.has_value()&& !lhs.has_value()) {
      return 0;
    }
    return lhs.value().get().timeIntervalSinceLastUpdate() > rhs.value().get().timeIntervalSinceLastUpdate();
  }
};

template <typename ContextT, std::size_t MaxDevicesCached = 10>
class ConcreteBLEDatabase : public BLEDatabase, public BLEDeviceDelegate /*, public std::enable_shared_from_this<ConcreteBLEDatabase<ContextT>>*/  {
public:
  static constexpr std::size_t MaxDevices = MaxDevicesCached;

  ConcreteBLEDatabase(ContextT& context) noexcept
  : ctx(context),
    delegates(),
    devices()
    HLOGGERINIT(context,"herald","ConcreteBLEDatabase")
  {
    ;
  }

  ConcreteBLEDatabase(const ConcreteBLEDatabase& from) = delete;
  ConcreteBLEDatabase(ConcreteBLEDatabase&& from) = delete;

  ~ConcreteBLEDatabase() noexcept = default;

  // BLE Database overrides

  void add(BLEDatabaseDelegate& delegate) noexcept override {
    delegates.add(std::optional(std::reference_wrapper(delegate)));
  }

  // Creation overrides
  BLEDevice& device(const BLEMacAddress& mac, const Data& advert/*, const RSSI& rssi*/) noexcept override {
    // Check by MAC first
    TargetIdentifier targetIdentifier(mac.underlyingData());
    auto results = matches([&targetIdentifier](const BLEDevice& d) {
      return d.identifier() == targetIdentifier;
    });
    if (results.size() != 0 && results[0].has_value()) {
      // HTDBG("DEVICE ALREADY KNOWN BY MAC");
      // Assume advert details are known already
      return results[0].value().get(); // TODO ensure we send back the latest, not just the first match
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
      if (0 != samePseudo.size() && samePseudo[0].has_value()) {
        // HTDBG("FOUND EXISTING DEVICE BY PSEUDO");
        return samePseudo[0].value().get();
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

  BLEDevice& device(const BLEMacAddress& mac, const BLEMacAddress& pseudo) noexcept override {
    auto samePseudo = matches([&pseudo](const BLEDevice& d) {
      return d.pseudoDeviceAddress() == pseudo;
    });
    if (0 == samePseudo.size()) {
      auto& ptr = device(TargetIdentifier(pseudo.underlyingData()));
      ptr.pseudoDeviceAddress(pseudo);
      return ptr;
    }
    // get most recent and clone, then attach
    auto comp = last_updated_descending();
    std::sort(samePseudo.begin(),samePseudo.end(), comp); // functional style
    BLEDevice& updatedDevice = samePseudo[0].value().get();
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
      if (delegate.has_value()) {
        delegate.value().get().bleDatabaseDidCreate(updatedDevice); // may be new with a new service
      }
    }
    return updatedDevice;
  }

  BLEDevice& device(const BLEMacAddress& mac) noexcept override {
    // HTDBG("device(BLEMacAddress)");
    // HTDBG((std::string)mac);
    return device(TargetIdentifier(mac.underlyingData()));
  }

  BLEDevice& device(const PayloadData& payloadData) noexcept override {
    // HTDBG("device(PayloadData)");
    // HTDBG(payloadData.toString());
    auto pti = TargetIdentifier(payloadData);
    auto results = matches([&pti,&payloadData](const BLEDevice& d) {
      if (d.identifier() == pti) {
        return true;
      }
      auto payload = d.payloadData();
      if (payload.size() == 0) {
        return false;
      }
      return payload==payloadData;
    });
    if (results.size() != 0 && results[0].has_value()) {
      return results[0].value().get(); // TODO ensure we send back the latest, not just the first match
    }
    BLEDevice& newDevice = devices[indexAvailable()];
    newDevice.reset(pti,*this);

    for (auto& delegate : delegates) {
      if (delegate.has_value()) {
        delegate.value().get().bleDatabaseDidCreate(newDevice);
      }
    }
    newDevice.payloadData(payloadData); // has to be AFTER create called
    device(newDevice,BLEDeviceAttribute::payloadData); // moved from BLEDevice.payloadData()
    return newDevice;
  }

  BLEDevice& device(const TargetIdentifier& targetIdentifier) noexcept override {
    // HTDBG("device(TargetIdentifier)");
    // HTDBG((std::string)targetIdentifier);
    auto results = matches([this,&targetIdentifier](const BLEDevice& d) {
      // HTDBG("device(TargetIdentifier) matches callback");
      // HTDBG(" Testing existing target identifier {} against new target identifier {}",(std::string)d.identifier(),(std::string)targetIdentifier);
      return d.identifier() == targetIdentifier;
    });
    // HTDBG("Got matches");
    // HTDBG(std::to_string(results.size()));
    if (results.size() != 0 && results[0].has_value()) {
      // HTDBG("Device for target identifier {} already exists",(std::string)targetIdentifier);
      return results[0].value().get(); // TODO ensure we send back the latest, not just the first match
    }
    HTDBG("New target identified: {}",(std::string)targetIdentifier);
    BLEDevice& newDevice = devices[indexAvailable()];
    newDevice.reset(targetIdentifier,*this);

    for (auto& delegate : delegates) {
      if (delegate.has_value()) {
        delegate.value().get().bleDatabaseDidCreate(newDevice);
      }
    }
    return newDevice;
  }
  
  // Introspection overrides
  std::size_t size() const noexcept override {
    std::size_t count = 0;
    for (auto& d : devices) {
      if (d.state() != BLEDeviceState::uninitialised) {
        ++count;
      }
    }
    return count;
  }

  BLEDeviceList matches(const std::function<bool(const BLEDevice&)>& matcher) noexcept override {
    // HTDBG("matches()");
    BLEDeviceList results;
    // in the absence of copy_if in C++20... Just copies the pointers not the objects
    // HTDBG("----");
    for (auto iter = devices.begin();iter != devices.end();++iter) {
      // HTDBG("ITER");
      // HTDBG((std::string)iter->identifier());
      // HTDBG(std::to_string((int)iter->state())); // Fails - like state is invalid...
      if (BLEDeviceState::uninitialised != iter->state() && matcher(*iter)) {
        // HTDBG("Match");
        results.add(std::reference_wrapper<BLEDevice>(*iter));
      }
      // HTDBG(".");
    }
    // HTDBG("Fin");
    return results;
  }

  /// Cannot name a function delete in C++. remove is common.
  void remove(const TargetIdentifier& targetIdentifier) noexcept override {
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
  void device(const BLEDevice& device, BLEDeviceAttribute didUpdate) noexcept override {
    // Update any internal DB state as necessary (E.g. payload received and its a duplicate as mac has rotated)
    if (BLEDeviceAttribute::payloadData == didUpdate) {
      // check for all devices with this payload that are NOT THIS device
      auto oldMacsForSamePayload = matches([device](auto& devRef) {
        return devRef.identifier() != device.identifier() && 
               devRef.payloadData().size() > 0 && devRef.payloadData() == device.payloadData();
      });
      for (auto& oldMacDevice : oldMacsForSamePayload) {
        if (oldMacDevice.has_value()) {
          remove(oldMacDevice.value().get().identifier());
        }
      }
    }

    // Now send update to delegates
    for (auto& delegate : delegates) {
      if (delegate.has_value()) {
        delegate.value().get().bleDatabaseDidUpdate(device, didUpdate); // TODO verify this is the right onward call
      }
    }
  }

private:
  void assignAdvertData(BLEDevice& newDevice, std::vector<BLEAdvertSegment>&& toMove, 
    const std::vector<BLEAdvertManufacturerData>& manuData) noexcept
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

  void remove(BLEDevice& toRemove) noexcept {
    // Don't call delete/update if this device has never been initialised
    if (toRemove.state() == BLEDeviceState::uninitialised) {
      return;
    }
    toRemove.state(BLEDeviceState::uninitialised);
    // TODO validate all other device data is reset
    for (auto& delegate : delegates) {
      if (delegate.has_value()) {
        delegate.value().get().bleDatabaseDidDelete(toRemove);
      }
    }
  }

  std::size_t indexAvailable() noexcept {
    for (std::size_t idx = 0;idx < devices.size();++idx) {
      auto& device = devices[idx];
      if (BLEDeviceState::uninitialised == device.state()) {
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
  BLEDatabaseDelegateList delegates;
  std::array<BLEDevice,MaxDevices> devices; // bool = in-use (not 'removed' from DB)

  HLOGGER(ContextT);
};

}
}

#endif