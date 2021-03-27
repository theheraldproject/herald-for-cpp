//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_CONCRETE_H
#define BLE_CONCRETE_H

#include "ble_database.h"
#include "ble_receiver.h"
#include "ble_sensor.h"
#include "ble_transmitter.h"
#include "ble_protocols.h"
#include "bluetooth_state_manager.h"
#include "ble_device_delegate.h"
#include "filter/ble_advert_parser.h"
#include "../payload/payload_data_supplier.h"
#include "../context.h"
#include "../data/sensor_logger.h"

#include <memory>
#include <vector>
#include <algorithm>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;

// NOTE THIS HEADER IS FOR ALL PLATFORMS. 
//      SPECIFIC PLATFORM DEFINITIONS ARE WITHIN SEVERAL C++ FILES
//      UNDER WINDOWS AND ZEPHYR SUB DIRECTORIES

class ConcreteBluetoothStateManager : public BluetoothStateManager, public std::enable_shared_from_this<BluetoothStateManager>  {
public:
  ConcreteBluetoothStateManager();
  ConcreteBluetoothStateManager(const ConcreteBluetoothStateManager& from) = delete;
  ConcreteBluetoothStateManager(ConcreteBluetoothStateManager&& from) = delete;
  ~ConcreteBluetoothStateManager();

  // Bluetooth State Manager overrides
  void add(std::shared_ptr<BluetoothStateManagerDelegate> delegate) override;
  BluetoothState state() override;
};

/// \brief Provides a callable that assists in ordering for most recently updated BLEDevice
struct last_updated_descending {
  bool operator()(const std::shared_ptr<BLEDevice>& a, const std::shared_ptr<BLEDevice>& b) {
    return a->timeIntervalSinceLastUpdate() > b->timeIntervalSinceLastUpdate(); // opposite order
  }
};

template <typename ContextT>
class ConcreteBLEDatabase : public BLEDatabase, public BLEDeviceDelegate, public std::enable_shared_from_this<ConcreteBLEDatabase<ContextT>>  {
public:
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

  void add(const std::shared_ptr<BLEDatabaseDelegate>& delegate) override {
    delegates.push_back(delegate);
  }

  // Creation overrides
  std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac, const Data& advert/*, const RSSI& rssi*/) override {
    // Check by MAC first
    TargetIdentifier targetIdentifier((Data)mac);
    auto results = matches([&targetIdentifier](const std::shared_ptr<BLEDevice>& d) {
      return d->identifier() == targetIdentifier;
    });
    if (results.size() != 0) {
      // HDBG("DEVICE ALREADY KNOWN BY MAC");
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
      // HDBG("Found Herald Android pseudo device address in advert");
      // Try to FIND by pseudo first
      BLEMacAddress pseudo(heraldDataSegments.front());
      auto samePseudo = matches([&pseudo](const std::shared_ptr<BLEDevice>& d) {
        return d->pseudoDeviceAddress() == pseudo;
      });
      if (0 != samePseudo.size()) {
        // HDBG("FOUND EXISTING DEVICE BY PSEUDO");
        return samePseudo.front();
      }
      // HDBG("CREATING NEW DEVICE BY MAC AND PSEUDO ONLY");
      // Now create new device with mac and pseudo
      auto newDevice = device(mac,pseudo);
      assignAdvertData(newDevice,std::move(segments), manuData);
      // newDevice->rssi(rssi);
      return newDevice;
    }

    // HDBG("CREATING NEW DEVICE BY MAC ONLY");

    // Now create a device just from a mac
    auto newDevice = device(targetIdentifier);
    // HDBG("Got new device");
    assignAdvertData(newDevice,std::move(segments), manuData);
    // newDevice->rssi(rssi);
    // HDBG("Assigned advert data");
    return newDevice;
  }

  std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac, const BLEMacAddress& pseudo) override {
    auto samePseudo = matches([&pseudo](const std::shared_ptr<BLEDevice>& d) {
      return d->pseudoDeviceAddress() == pseudo;
    });
    if (0 == samePseudo.size()) {
      auto ptr = device(TargetIdentifier((Data)pseudo));
      ptr->pseudoDeviceAddress(pseudo);
      return ptr;
    }
    // get most recent and clone, then attach
    auto comp = last_updated_descending();
    std::sort(samePseudo.begin(),samePseudo.end(), comp); // functional style
    auto newDevice = std::make_shared<BLEDevice>(*samePseudo.front()); // copy ctor used
    // TODO support calling card
    // auto toShare = shareDataAcrossDevices(pseudo);
    // if (toShare.has_value()) {
    //   newDevice.payloadData(toShare);
    // }
    
    // Has pseudo address so must be android
    newDevice->operatingSystem(BLEDeviceOperatingSystem::android);

    // register new device discovery date
    newDevice->registerDiscovery(Date());

    devices.push_back(newDevice);
    for (auto delegate : delegates) {
      delegate->bleDatabaseDidCreate(newDevice);
    }
    return newDevice;
  }

  std::shared_ptr<BLEDevice> device(const BLEMacAddress& mac) override {
    return device(TargetIdentifier((Data)mac));
  }

  std::shared_ptr<BLEDevice> device(const PayloadData& payloadData) override {
    auto results = matches([&payloadData](const std::shared_ptr<BLEDevice>& d) {
      auto payload = d->payloadData();
      if (!payload.has_value()) {
        return false;
      }
      return (*payload)==payloadData;
    });
    if (results.size() != 0) {
      return results.front(); // TODO ensure we send back the latest, not just the first match
    }
    std::shared_ptr<BLEDevice> newDevice = std::make_shared<BLEDevice>(
      TargetIdentifier(payloadData), this->shared_from_this());
    devices.push_back(newDevice);
    for (auto delegate : delegates) {
      delegate->bleDatabaseDidCreate(newDevice);
    }
    return newDevice;
  }
  std::shared_ptr<BLEDevice> device(const TargetIdentifier& targetIdentifier) override {
    auto results = matches([&targetIdentifier](const std::shared_ptr<BLEDevice>& d) {
      return d->identifier() == targetIdentifier;
    });
    if (results.size() != 0) {
      return results.front(); // TODO ensure we send back the latest, not just the first match
    }
    HTDBG("New target identified: {}",(std::string)targetIdentifier);
    std::shared_ptr<BLEDevice> newDevice = std::make_shared<BLEDevice>(
      targetIdentifier, this->shared_from_this());
    devices.push_back(newDevice);
    for (auto delegate : delegates) {
      delegate->bleDatabaseDidCreate(newDevice);
    }
    return newDevice;
  }
  
  // Introspection overrides
  std::size_t size() const override {
    return devices.size();
  }

  std::vector<std::shared_ptr<BLEDevice>> matches(
    const std::function<bool(const std::shared_ptr<BLEDevice>&)>& matcher) const override {
    std::vector<std::shared_ptr<BLEDevice>> results;
    // in the absence of copy_if in C++20... Just copies the pointers not the objects
    for (auto& d : devices) {
      if (matcher(d)) {
        results.push_back(d);
      }
    }
    return results;
  }

  // std::vector<std::shared_ptr<BLEDevice>> devices() const override;

  /// Cannot name a function delete in C++. remove is common.
  void remove(const TargetIdentifier& targetIdentifier) override {
    auto found = std::find_if(devices.begin(),devices.end(),
      [&targetIdentifier](std::shared_ptr<BLEDevice>& d) -> bool {
        return d->identifier() == targetIdentifier;
      }
    );
    if (found != devices.end()) {
      std::shared_ptr<BLEDevice> toRemove = *found;
      devices.erase(found);
      for (auto& delegate : delegates) {
        delegate->bleDatabaseDidDelete(toRemove);
      }
    }
  }

  // std::optional<PayloadSharingData> payloadSharingData(const std::shared_ptr<BLEDevice>& peer) override;

  // BLE Device Delegate overrides
  void device(const std::shared_ptr<BLEDevice>& device, BLEDeviceAttribute didUpdate) override {
    // TODO update any internal DB state as necessary (E.g. deletion)
    for (auto& delegate : delegates) {
      delegate->bleDatabaseDidUpdate(device, didUpdate); // TODO verify this is the right onward call
    }
  }

private:
  void assignAdvertData(std::shared_ptr<BLEDevice>& newDevice, std::vector<BLEAdvertSegment>&& toMove, 
    const std::vector<BLEAdvertManufacturerData>& manuData)
  {
    newDevice->advertData(std::move(toMove));

    // If it's an apple device, check to see if its on our ignore list
    auto appleDataSegments = BLEAdvertParser::extractAppleManufacturerSegments(manuData);
    if (0 != appleDataSegments.size()) {
      HTDBG("Found apple device");
      // HTDBG((std::string)mac);
      newDevice->operatingSystem(BLEDeviceOperatingSystem::ios);
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
        newDevice->ignore(true);
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

  ContextT& ctx;
  std::vector<std::shared_ptr<BLEDatabaseDelegate>> delegates;
  std::vector<std::shared_ptr<BLEDevice>> devices;

  HLOGGER(ContextT);
};

/**
 * Acts as the main object to control the receiver, transmitter, and database instances
 */
template <typename ContextT>
class ConcreteBLESensor : public BLESensor, public BLEDatabaseDelegate, 
  public BluetoothStateManagerDelegate, public std::enable_shared_from_this<ConcreteBLESensor<ContextT>>  {
public:
  ConcreteBLESensor(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ConcreteBLESensor(const ConcreteBLESensor& from) = delete;
  ConcreteBLESensor(ConcreteBLESensor&& from) = delete;
  ~ConcreteBLESensor();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier);
  bool immediateSendAll(Data data);

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override;
  void start() override;
  void stop() override;

  // Database overrides
  void bleDatabaseDidCreate(const std::shared_ptr<BLEDevice>& device) override;
  void bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice>& device, const BLEDeviceAttribute attribute) override;
  void bleDatabaseDidDelete(const std::shared_ptr<BLEDevice>& device) override;

  // Bluetooth state manager delegate overrides
  void bluetoothStateManager(BluetoothState didUpdateState) override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // unique as this is handled internally for all platforms by Herald
};

template <typename ContextT>
class ConcreteBLEReceiver : public BLEReceiver, public HeraldProtocolV1Provider, public std::enable_shared_from_this<ConcreteBLEReceiver<ContextT>> {
public:
  ConcreteBLEReceiver(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLEReceiver(const ConcreteBLEReceiver& from) = delete;
  ConcreteBLEReceiver(ConcreteBLEReceiver&& from) = delete;
  ~ConcreteBLEReceiver();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override;
  bool immediateSendAll(Data data) override;

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override;
  void start() override;
  void stop() override;

  // Herald V1 protocol provider overrides
  // C++17 CALLBACK VERSION:-
  // void openConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void closeConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void serviceDiscovery(Activity, CompletionCallback) override;
  // void readPayload(Activity, CompletionCallback) override;
  // void immediateSend(Activity, CompletionCallback) override;
  // void immediateSendAll(Activity, CompletionCallback) override;
  
  // NON C++17 VERSION:-
  bool openConnection(const TargetIdentifier& toTarget) override;
  bool closeConnection(const TargetIdentifier& toTarget) override;
  void restartScanningAndAdvertising() override;
  std::optional<Activity> serviceDiscovery(Activity) override;
  std::optional<Activity> readPayload(Activity) override;
  std::optional<Activity> immediateSend(Activity) override;
  std::optional<Activity> immediateSendAll(Activity) override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

template <typename ContextT>
class ConcreteBLETransmitter : public BLETransmitter, public std::enable_shared_from_this<ConcreteBLETransmitter<ContextT>> {
public:
  ConcreteBLETransmitter(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLETransmitter(const ConcreteBLETransmitter& from) = delete;
  ConcreteBLETransmitter(ConcreteBLETransmitter&& from) = delete;
  ~ConcreteBLETransmitter();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override;
  void start() override;
  void stop() override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

} // end namespace
} // end namespace

#endif