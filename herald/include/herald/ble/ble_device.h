//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DEVICE_H
#define BLE_DEVICE_H

#include "ble_tx_power.h"
#include "ble_mac_address.h"

#include "../device.h"

#include "../datatype/payload_data.h"
#include "../datatype/payload_sharing_data.h"
#include "../datatype/immediate_send_data.h"
#include "../datatype/target_identifier.h"
#include "../datatype/time_interval.h"
#include "../datatype/date.h"
#include "../datatype/uuid.h"

#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class BLEDeviceDelegate; // fwd decl

enum class BLEDeviceAttribute : int {
  peripheral, state, operatingSystem, payloadData, rssi, txPower, immediateSendData
};

enum class BLEDeviceOperatingSystem : int {
  android_tbc, android, ios_tbc, ios, ignore, shared, unknown
};

enum class BLEDeviceState : int {
  connecting, connected, disconnected
};

class BLEDevice : public Device, public std::enable_shared_from_this<BLEDevice> {
public:
  BLEDevice(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> delegate, const Date& created = Date());
  ~BLEDevice();

  const TargetIdentifier& identifier() const override;
  Date created() const override;

  // basic descriptors
  std::string description() const;
  operator std::string() const;

  // timing related getters
  TimeInterval timeIntervalSinceLastUpdate() const override;
  TimeInterval timeIntervalSinceConnected() const;
  TimeInterval timeIntervalSinceLastPayloadDataUpdate() const;
  TimeInterval timeIntervalSinceLastWritePayloadSharing() const;
  TimeInterval timeIntervalSinceLastWritePayload() const;
  TimeInterval timeIntervalSinceLastWriteRssi() const;

  // property getters and setters
  std::optional<UUID> signalCharacteristic() const;
  void signalCharacteristic(UUID newChar);

  std::optional<UUID> payloadCharacteristic() const;
  void payloadCharacteristic(UUID newChar);

  std::optional<BLEMacAddress> pseudoDeviceAddress() const;
  void pseudoDeviceAddress(BLEMacAddress newAddress);

  std::optional<BLEDeviceState> state() const;
  void state(BLEDeviceState newState);
  std::optional<BLEDeviceOperatingSystem> operatingSystem() const;
  void operatingSystem(BLEDeviceOperatingSystem newOS);

  std::optional<PayloadData> payloadData() const;
  void payloadData(PayloadData newPayloadData);

  std::optional<ImmediateSendData> immediateSendData() const;
  void immediateSendData(ImmediateSendData toSend);

  std::optional<RSSI> rssi() const;
  void rssi(RSSI newRSSI);

  std::optional<BLETxPower> txPower() const;
  void txPower(BLETxPower newPower);

  bool receiveOnly() const;
  void receiveOnly(bool newReceiveOnly);

  // State engine methods
  bool ignore() const;
  void ignore(bool newIgnore);
  void invalidateCharacteristics();
  void registerDiscovery(Date& at); // ALWAYS externalise time (now())
  void registerWritePayload(Date& at); // ALWAYS externalise time (now())
  void registerWritePayloadSharing(Date& at); // ALWAYS externalise time (now())
  void registerWriteRssi(Date& at); // ALWAYS externalise time (now())
  
private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif