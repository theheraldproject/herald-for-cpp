//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/signal_characteristic_data.h"
#include "herald/ble/ble_sensor_configuration.h"

#include <vector>
#include <optional>

namespace herald {
namespace datatype {
namespace SignalCharacteristicData {

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
encodeWriteRssi(const BLESensorConfiguration& config,const RSSI& rssi) noexcept {
  int r = rssi.intValue();
  std::vector<std::byte> vec(3);
  vec.push_back(config.signalCharacteristicActionWriteRSSI);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  return std::optional<Data>(Data(std::move(vec))); // constructs the optional with a value
}

std::optional<RSSI>
decodeWriteRSSI(const BLESensorConfiguration& config,const Data& data) noexcept {
  if (signalDataActionCode(data) != config.signalCharacteristicActionWriteRSSI) {
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
  return std::optional<RSSI>(RSSI{rssi}); // constructs the optional with a value
}

std::optional<Data>
encodeWritePayload(const BLESensorConfiguration& config,const PayloadData& payloadData) noexcept {
  int r = (int)payloadData.size();
  std::vector<std::byte> vec(3 + r);
  vec.push_back(config.signalCharacteristicActionWritePayload);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  Data d(std::move(vec));
  d.append(payloadData);
  return std::optional<Data>(d); // constructs the optional with a value
}

std::optional<PayloadData>
decodeWritePayload(const BLESensorConfiguration& config,const Data& data) noexcept {
  if (signalDataActionCode(data) != config.signalCharacteristicActionWritePayload) {
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
  if (data.size() != (3 + std::size_t(payloadDataCount))) {
    return {};
  }
  return std::optional<PayloadData>(PayloadData(data.subdata(3))); // constructs the optional with a value
}

std::optional<Data>
encodeWritePayloadSharing(const BLESensorConfiguration& config,const PayloadSharingData& payloadSharingData) noexcept {
  int r = payloadSharingData.rssi.intValue();
  int r2 = (int)payloadSharingData.data.size();
  std::vector<std::byte> vec(5 + r2);
  vec.push_back(config.signalCharacteristicActionWritePayloadSharing);
  vec.push_back(std::byte(r)); // force least significant bit
  vec.push_back(std::byte(r >> 8)); // msb
  vec.push_back(std::byte(r2)); // force least significant bit
  vec.push_back(std::byte(r2 >> 8)); // msb
  Data d(std::move(vec));
  d.append(payloadSharingData.data);
  return std::optional<Data>(d); // constructs the optional with a value
}

std::optional<PayloadSharingData>
decodeWritePayloadSharing(const BLESensorConfiguration& config,const Data& data) noexcept {
  if (signalDataActionCode(data) != config.signalCharacteristicActionWritePayloadSharing) {
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
  if (data.size() != (5 + std::size_t(payloadDataCount))) {
    return {};
  }
  Data d = data.subdata(5);
  RSSI rssi(rssiValue);
  PayloadSharingData pd{std::move(rssi), std::move(data)};
  return std::optional<PayloadSharingData>(pd); // constructs the optional with a value
}

std::optional<Data>
encodeImmediateSend(const BLESensorConfiguration& config,const ImmediateSendData& immediateSendData) noexcept {
  int r = (int)immediateSendData.size();
  std::vector<std::byte> vec(3 + r);
  vec.push_back(config.signalCharacteristicActionWriteImmediate);
  vec.push_back(static_cast<std::byte>(r)); // force least significant bit
  vec.push_back(static_cast<std::byte>(r >> 8)); // msb
  Data d(std::move(vec));
  d.append(immediateSendData);
  return std::optional<Data>(d); // constructs the optional with a value
}

std::optional<ImmediateSendData>
decodeImmediateSend(const BLESensorConfiguration& config,const Data& data) noexcept {
  if (signalDataActionCode(data) != config.signalCharacteristicActionWriteImmediate) {
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
  if (data.size() != (3 + std::size_t(payloadDataCount))) {
    return {};
  }
  return std::optional<ImmediateSendData>(ImmediateSendData(data.subdata(3))); // constructs the optional with a value
}

SignalCharacteristicDataType
detect(const BLESensorConfiguration& config,const Data& data) noexcept {
  auto val = signalDataActionCode(data);
  if (config.signalCharacteristicActionWriteRSSI == val) {
    return SignalCharacteristicDataType::rssi;
  } else if (config.signalCharacteristicActionWritePayload == val) {
    return SignalCharacteristicDataType::payload;
  } else if (config.signalCharacteristicActionWritePayloadSharing == val) {
    return SignalCharacteristicDataType::payloadSharing;
  } else if (config.signalCharacteristicActionWriteImmediate == val) {
    return SignalCharacteristicDataType::immediateSend;
  } else {
    return SignalCharacteristicDataType::unknown;
  }
}

} // end namespace
} // end namespace
} // end namespace
