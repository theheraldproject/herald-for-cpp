//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_SENSOR_CONFIGURATION_H
#define BLE_SENSOR_CONFIGURATION_H

#include "../datatype/time_interval.h"
#include "../datatype/uuid.h"

namespace herald {
namespace ble {
  
using namespace herald::datatype;

/// Defines BLE sensor configuration data, e.g. service and characteristic UUIDs
struct BLESensorConfiguration {
  BLESensorConfiguration();
  BLESensorConfiguration(const BLESensorConfiguration& other);
  ~BLESensorConfiguration() = default;

  // MARK:- BLE service and characteristic UUID, and manufacturer ID

  /// Service UUID for beacon service. This is a fixed UUID to enable iOS devices to find each other even
  /// in background mode. Android devices will need to find Apple devices first using the manufacturer code
  /// then discover services to identify actual beacons.
  /// - Service and characteristic UUIDs are V4 UUIDs that have been randomly generated and tested
  /// for uniqueness by conducting web searches to ensure it returns no results.
  UUID serviceUUID;
  /// Signaling characteristic for controlling connection between peripheral and central, e.g. keep each other from suspend state
  /// - Characteristic UUID is randomly generated V4 UUIDs that has been tested for uniqueness by conducting web searches to ensure it returns no results.
  UUID androidSignalCharacteristicUUID;
  /// Signaling characteristic for controlling connection between peripheral and central, e.g. keep each other from suspend state
  /// - Characteristic UUID is randomly generated V4 UUIDs that has been tested for uniqueness by conducting web searches to ensure it returns no results.
  UUID iosSignalCharacteristicUUID;
  /// Primary payload characteristic (read) for distributing payload data from peripheral to central, e.g. identity data
  /// - Characteristic UUID is randomly generated V4 UUIDs that has been tested for uniqueness by conducting web searches to ensure it returns no results.
  UUID payloadCharacteristicUUID;
  
  /// Manufacturer data is being used on Android to store pseudo device address
  /// - Pending update to dedicated ID
  int manufacturerIdForSensor;
  /// BLE advert manufacturer ID for Apple, for scanning of background iOS devices
  int manufacturerIdForApple;

  // MARK:- BLE signal characteristic action codes

  /// Signal characteristic action code for write payload, expect 1 byte action code followed by 2 byte little-endian Int16 integer value for payload data length, then payload data
  std::byte signalCharacteristicActionWritePayload;
  /// Signal characteristic action code for write RSSI, expect 1 byte action code followed by 4 byte little-endian Int32 integer value for RSSI value
  std::byte signalCharacteristicActionWriteRSSI;
  /// Signal characteristic action code for write payload, expect 1 byte action code followed by 2 byte little-endian Int16 integer value for payload sharing data length, then payload sharing data
  std::byte signalCharacteristicActionWritePayloadSharing;
  /// Arbitrary immediate write
  std::byte signalCharacteristicActionWriteImmediate;

  // MARK:- App configurable BLE features

  /// Log level for BLESensor
  //SensorLoggerLevel logLevel = SensorLoggerLevel.debug;

  /// Payload update at regular intervals, in addition to default HERALD communication process.
  /// - Use this to enable regular payload reads according to app payload lifespan.
  /// - Set to .never to disable this function.
  /// - Payload updates are reported to SensorDelegate as didRead.
  /// - Setting take immediate effect, no need to restart BLESensor, can also be applied while BLESensor is active.
  TimeInterval payloadDataUpdateTimeInterval;

  /// Expiry time for shared payloads, to ensure only recently seen payloads are shared
  TimeInterval payloadSharingExpiryTimeInterval;

  /// Advert refresh time interval
  TimeInterval advertRefreshTimeInterval;

  /// Remove peripheral records that haven't been updated for some time.
  /// - Herald aims to maintain a regular "connection" to all peripherals to gather precise proximity and duration data for all peripheral records.
  /// - A regular connection in this context means frequent data sampling that may or may not require an actual connection.
  /// - For example, RSSI measurements are taken from adverts, thus do not require an active connection; even the connection on iOS is just an illusion for ease of understanding.
  /// - A peripheral record stops updating if the device has gone out of range, therefore the record can be deleted to reduce workload.
  /// - Upper bound : Set this value to iOS Bluetooth address rotation period (roughly 15 minutes) to maximise continuity when devices go out of range, then return back in range (connection resume period = 15 mins max).
  /// - Lower bound : Set this value to Android scan-process period (roughly 2 minutes) to minimise workload, but iOS connection resume will be more reliant on re-discovery (connection resume period = 2 mins or more dependent on external factors).
  /// - iOS-iOS connections may resume beyond the set interval value if the addresses have not changed, due to other mechanisms in Herald.
  TimeInterval peripheralCleanInterval;

  /// Connection management
  /// Max connections - since v1.2 (allowing multiple connections on Android and C++)
  int maxBluetoothConnections; // Same as NRF 52840 max connections

  // Does this Herald application support advertising?
  bool advertisingEnabled;
  // Does this Herald application support scanning? (Simple Venue Beacons don't)
  bool scanningEnabled;


}; // end struct

} // end namespace
} // end namespace

#endif