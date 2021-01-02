//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_DEVICE_H
#define BLE_DEVICE_H

#include "../device.h"

#include "../datatype/payload_data.h"
#include "../datatype/payload_sharing_data.h"
#include "../datatype/immediate_send_data.h"
#include "../datatype/target_identifier.h"
#include "../datatype/time_interval.h"
#include "../datatype/date.h"

#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class BLEDeviceDelegate; // fwd decl

enum class BLEDeviceAttribute : int {
  peripheral, state, operatingSystem, payloadData, rssi, txPower
};

enum class BLEDeviceOperatingSystem : int {
  android_tbc, android, ios_tbc, ios, ignore, shared, unknown
};

enum class BLEDeviceState : int {
  connecting, connected, disconnected
};

class BLEDevice : public Device {
public:
  BLEDevice(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> delegate);
  //BLEDevice(const BLEDevice& device, const BluetoothDevice& bluetoothDevice);
  ~BLEDevice() = default;

  // basic descriptors
  std::string description() const;
  std::string toString() const; // TODO change these to c++ conversion operators

  // timing related getters
  std::optional<TimeInterval> timeIntervalSinceLastUpdate() const override;
  std::optional<TimeInterval> timeIntervalSinceConnected() const;
  std::optional<TimeInterval> timeIntervalSinceLastPayloadDataUpdate() const;
  std::optional<TimeInterval> timeIntervalSinceLastWritePayloadSharing() const;
  std::optional<TimeInterval> timeIntervalSinceLastWritePayload() const;
  std::optional<TimeInterval> timeIntervalSinceLastWriteRssi() const;

  // property getters and setters
  //std::optional<PseudoDeviceAddress> pseudoDeviceAddress() const;
  //void pseudoDeviceAddress(PseudoDeviceAddress newAddress);
  //std::optional<BluetothDevice> peripheral() const;
  //void peripheral(BluetoothDevice newPeripheral);
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

  //std::optional<BLETxPower> txPower() const;
  //void txPower(BLETxPower newPower);

  bool receiveOnly() const;
  void receiveOnly(bool newReceiveOnly);

  //std::optional<BluetoothGattCharacteristic> signalCharacteristic() const; // TODO local OS alternative
  //void signalCharacteristic(BluetoothGattCharacteristic newChar);
  
  //std::optional<BluetoothGattCharacteristic> payloadCharacteristic() const; // TODO local OS alternative
  //void payloadCharacteristic(BluetoothGattCharacteristic newChar);

  // State engine methods
  bool ignore() const;
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