//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_sensor_configuration.h"

namespace herald {
namespace ble {

BLESensorConfiguration::BLESensorConfiguration()
  : serviceUUID(UUID::fromString("428132af-4746-42d3-801e-4572d65bfd9b")),
    androidSignalCharacteristicUUID(UUID::fromString("f617b813-092e-437a-8324-e09a80821a11")),
    iosSignalCharacteristicUUID(UUID::fromString("0eb0d5f2-eae4-4a9a-8af3-a4adb02d4363")),
    payloadCharacteristicUUID(UUID::fromString("3e98c0f8-8f05-4829-a121-43e38f8933e7")),
    manufacturerIdForSensor(65530),
    manufacturerIdForApple(76),
    signalCharacteristicActionWritePayload(std::byte(1)),
    signalCharacteristicActionWriteRSSI(std::byte(2)),
    signalCharacteristicActionWritePayloadSharing(std::byte(3)),
    signalCharacteristicActionWriteImmediate(std::byte(4)),
    payloadDataUpdateTimeInterval(TimeInterval::never()),
    payloadSharingExpiryTimeInterval(TimeInterval::minutes(5)),
    advertRefreshTimeInterval(TimeInterval::minutes(15)),
    maxBluetoothConnections(20),
    advertisingEnabled(true),
    scanningEnabled(true)
{
  ;
}

BLESensorConfiguration::BLESensorConfiguration(const BLESensorConfiguration& other)
  : serviceUUID(other.serviceUUID),
    androidSignalCharacteristicUUID(other.androidSignalCharacteristicUUID),
    iosSignalCharacteristicUUID(other.iosSignalCharacteristicUUID),
    payloadCharacteristicUUID(other.payloadCharacteristicUUID),
    manufacturerIdForSensor(other.manufacturerIdForSensor),
    manufacturerIdForApple(other.manufacturerIdForApple),
    signalCharacteristicActionWritePayload(other.signalCharacteristicActionWritePayload),
    signalCharacteristicActionWriteRSSI(other.signalCharacteristicActionWriteRSSI),
    signalCharacteristicActionWritePayloadSharing(other.signalCharacteristicActionWritePayloadSharing),
    signalCharacteristicActionWriteImmediate(other.signalCharacteristicActionWriteImmediate),
    payloadDataUpdateTimeInterval(other.payloadDataUpdateTimeInterval),
    payloadSharingExpiryTimeInterval(other.payloadSharingExpiryTimeInterval),
    advertRefreshTimeInterval(other.advertRefreshTimeInterval),
    maxBluetoothConnections(other.maxBluetoothConnections),
    advertisingEnabled(other.advertisingEnabled),
    scanningEnabled(other.scanningEnabled)
{
  ;
}

}
}