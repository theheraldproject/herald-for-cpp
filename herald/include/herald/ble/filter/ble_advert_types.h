//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_ADVERT_TYPES_H
#define BLE_ADVERT_TYPES_H

#include "../../datatype/data.h"

#include <vector>
// #include <string>
// #include <cstdint>

namespace herald {
namespace ble {
namespace filter {

using namespace herald::datatype;

// high level types

// We use the below to convert BLEAdvertSegmentType to int 
// (or indeed any enum class to any base class)
template <typename T>
constexpr auto to_integral(T e) { return static_cast<std::underlying_type_t<T>>(e); }

/// BLE Advert types - Note: We only list those we use in Herald for some reason
/// See https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/
enum class BLEAdvertSegmentType : int {
  unknown = 0x00, // Valid - this number is not assigned
  flags = 0x01,
  serviceUUID16IncompleteList = 0x02,
  serviceUUID16CompleteList = 0x03,
  serviceUUID32IncompleteList = 0x04,
  serviceUUID32CompleteList = 0x05,
  serviceUUID128IncompleteList = 0x06,
  serviceUUID128CompleteList = 0x07,
  deviceNameShortened = 0x08,
  deviceNameComplete = 0x09,
  txPowerLevel = 0x0A,
  deviceClass = 0x0D,
  simplePairingHash = 0x0E,
  simplePairingRandomiser = 0x0F,
  deviceID = 0x10,
  meshMessage = 0x2A,
  meshBeacon = 0x2B,
  bigInfo = 0x2C,
  broadcastCode = 0x2D,
  manufacturerData = 0xFF
};

BLEAdvertSegmentType typeFor(int code);
// BLEAdvertSegmentType typeFor(const std::string& name);

struct BLEAdvertSegment {
  BLEAdvertSegmentType type;
  Data data;
  BLEAdvertSegment(BLEAdvertSegmentType t,Data&& d) : type(t), data(d) {};
  BLEAdvertSegment(const BLEAdvertSegment&) = default;
  BLEAdvertSegment(BLEAdvertSegment&&) = default;
  BLEAdvertSegment& operator=(const BLEAdvertSegment&) = default;
  BLEAdvertSegment& operator=(BLEAdvertSegment&&) = default;
};

struct BLEScanResponseData {
  std::size_t dataLength;
  std::vector<BLEAdvertSegment> segments;
  BLEScanResponseData(std::size_t dl, std::vector<BLEAdvertSegment>&& segs) :
    dataLength(dl), segments(segs) {};
  BLEScanResponseData(const BLEScanResponseData&) = default;
  BLEScanResponseData(BLEScanResponseData&&) = default;
};

enum class BLEAdvertManufacturers : uint16_t {
  // NOTE: Little endian actual values at this point
  apple = 0x004c, // TODO patch the Android extractAppleManuSegment function too
  heraldUnregistered = 0xfaff
};

// low level types
struct BLEAdvertManufacturerData {
  std::uint16_t manufacturer;
  Data data;
  BLEAdvertManufacturerData(std::uint16_t code, Data&& d) : manufacturer(code), data(d) {};
  BLEAdvertManufacturerData(const BLEAdvertManufacturerData&) = default;
  BLEAdvertManufacturerData(BLEAdvertManufacturerData&&) = default;
};

struct BLEAdvertAppleManufacturerSegment {
  std::uint8_t type;
  Data data;
  BLEAdvertAppleManufacturerSegment(std::uint8_t t, Data&& d) : type(t), data(d) {};
  BLEAdvertAppleManufacturerSegment(const BLEAdvertAppleManufacturerSegment&) = default;
  BLEAdvertAppleManufacturerSegment(BLEAdvertAppleManufacturerSegment&&) = default;
};

}
}
}

#endif