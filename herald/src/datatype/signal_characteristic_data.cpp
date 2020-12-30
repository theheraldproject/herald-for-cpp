//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/signal_characteristic_data.h"
#include "herald/ble/ble_sensor_configuration.h"

#include <vector>

namespace herald {
namespace datatype {

using namespace herald::ble;

// PRIVATE METHODS
std::byte
signalDataActionCode(const Data& signalData) {
  if (signalData.size() == 0) {
    return std::byte(0);
  }
  return signalData.at(0);
}

int
int16(const Data& data, std::size_t index, bool& success) {
  if (index < data.size() - 1) { // TODO TEST size() boundary limits - as it's two bytes, little endian
    int v = (((int)data.at(index)) << 8) | (int)data.at(index + 1);
    success = true;
    return v;
  }
  success = false;
  return 0;
}


// HEADER PUBLIC METHODS

std::optional<Data>
SignalCharacteristicData::encodeWriteRssi(const RSSI& rssi) noexcept {
  int r = rssi.intValue();
  std::vector<std::byte> vec(3);
  vec.push_back(BLESensorConfiguration::signalCharacteristicActionWriteRSSI);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  return std::move(Data(std::move(vec))); // constructs the optional with a value
}

std::optional<RSSI>
SignalCharacteristicData::decodeWriteRSSI(const Data& data) noexcept {
  if (signalDataActionCode(data) != BLESensorConfiguration::signalCharacteristicActionWriteRSSI) {
    return {};
  }
  if (data.size() != 3) {
    return {};
  }
  bool success = true;
  int rssi = int16(data,1, success); // idx 1 & 2 (little endian)
  if (!success) {
    return {};
  }
  return std::optional(rssi); // constructs the optional with a value
}

std::optional<Data>
SignalCharacteristicData::encodeWritePayload(const PayloadData& payloadData) noexcept {
  int r = (int)payloadData.size();
  std::vector<std::byte> vec(3 + r);
  vec.push_back(BLESensorConfiguration::signalCharacteristicActionWritePayload);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  Data d(std::move(vec));
  d.append(payloadData);
  return std::move(d); // constructs the optional with a value
}

std::optional<PayloadData>
SignalCharacteristicData::decodeWritePayload(const Data& data) noexcept {
  if (signalDataActionCode(data) != BLESensorConfiguration::signalCharacteristicActionWritePayload) {
    return {};
  }
  if (data.size() < 3) {
    return {};
  }
  bool success = true;
  int payloadDataCount = int16(data, 1, success); // idx 1 & 2 (little endian)
  if (!success) {
    return {};
  }
  if (data.size() != (3 + payloadDataCount)) {
    return {};
  }
  return std::move(PayloadData(data.subdata(3))); // constructs the optional with a value
}

std::optional<Data>
SignalCharacteristicData::encodeWritePayloadSharing(const PayloadSharingData& payloadSharingData) noexcept {
  int r = payloadSharingData.rssi.intValue();
  int r2 = (int)payloadSharingData.data.size();
  std::vector<std::byte> vec(5 + r2);
  vec.push_back(BLESensorConfiguration::signalCharacteristicActionWritePayloadSharing);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  vec.push_back(std::byte(r2)); // force least significant bit
  vec.push_back(std::byte(r2 >> 8)); // msb
  Data d(std::move(vec));
  d.append(payloadSharingData.data);
  return std::move(d); // constructs the optional with a value
}

std::optional<PayloadSharingData>
SignalCharacteristicData::decodeWritePayloadSharing(const Data& data) noexcept {
  if (signalDataActionCode(data) != BLESensorConfiguration::signalCharacteristicActionWritePayloadSharing) {
    return {};
  }
  if (data.size() < 5) {
    return {};
  }
  bool success = true;
  int rssiValue = int16(data, 1, success);
  if (!success) {
    return {};
  }
  int payloadDataCount = int16(data, 3, success); // idx 3 & 4 (little endian)
  if (!success) {
    return {};
  }
  if (data.size() != (5 + payloadDataCount)) {
    return {};
  }
  Data d = data.subdata(5);
  RSSI rssi(rssiValue);
  PayloadSharingData pd{std::move(rssi), std::move(data)};
  return std::move(pd); // constructs the optional with a value
}

std::optional<Data>
SignalCharacteristicData::encodeImmediateSend(const ImmediateSendData& immediateSendData) noexcept {
  int r = (int)immediateSendData.size();
  std::vector<std::byte> vec(3 + r);
  vec.push_back(BLESensorConfiguration::signalCharacteristicActionWriteImmediate);
  vec.push_back(static_cast<std::byte>(r)); // force least significant bit
  vec.push_back(static_cast<std::byte>(r >> 8)); // msb
  Data d(std::move(vec));
  d.append(immediateSendData);
  return std::move(d); // constructs the optional with a value
}

std::optional<ImmediateSendData>
SignalCharacteristicData::decodeImmediateSend(const Data& data) noexcept {
  if (signalDataActionCode(data) != BLESensorConfiguration::signalCharacteristicActionWriteImmediate) {
    return {};
  }
  if (data.size() < 3) {
    return {};
  }
  bool success = true;
  int payloadDataCount = int16(data, 1, success); // idx 1 & 2 (little endian)
  if (!success) {
    return {};
  }
  if (data.size() != (3 + payloadDataCount)) {
    return {};
  }
  return std::move(ImmediateSendData(data.subdata(3))); // constructs the optional with a value
}

SignalCharacteristicDataType
SignalCharacteristicData::detect(const Data& data) noexcept {
  switch (signalDataActionCode(data)) {
    case BLESensorConfiguration::signalCharacteristicActionWriteRSSI:
      return SignalCharacteristicDataType::rssi;
    case BLESensorConfiguration::signalCharacteristicActionWritePayload:
      return SignalCharacteristicDataType::payload;
    case BLESensorConfiguration::signalCharacteristicActionWritePayloadSharing:
      return SignalCharacteristicDataType::payloadSharing;
    case BLESensorConfiguration::signalCharacteristicActionWriteImmediate:
      return SignalCharacteristicDataType::immediateSend;
    default:
      return SignalCharacteristicDataType::unknown;
  }
}

} // end namespace
} // end namespace
