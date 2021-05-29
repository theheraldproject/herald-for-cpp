//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIGNAL_CHARACTERISTIC_DATA_H
#define SIGNAL_CHARACTERISTIC_DATA_H

#include "data.h"
#include "rssi.h"
#include "payload_data.h"
#include "payload_sharing_data.h"
#include "immediate_send_data.h"
#include "../ble/ble_sensor_configuration.h"

#include <optional>

namespace herald {
namespace datatype {

enum class SignalCharacteristicDataType : short {
  rssi, payload, payloadSharing, immediateSend, unknown
};

namespace SignalCharacteristicData {
using namespace herald::ble;

[[maybe_unused]]
std::optional<Data> encodeWriteRssi(const BLESensorConfiguration& config,const RSSI& rssi) noexcept;

[[maybe_unused]]
std::optional<RSSI> decodeWriteRSSI(const BLESensorConfiguration& config,const Data& data) noexcept;

[[maybe_unused]]
std::optional<Data> encodeWritePayload(const BLESensorConfiguration& config,const PayloadData& payloadData) noexcept;

[[maybe_unused]]
std::optional<PayloadData> decodeWritePayload(const BLESensorConfiguration& config,const Data& data) noexcept;

[[maybe_unused]]
std::optional<Data> encodeWritePayloadSharing(const BLESensorConfiguration& config,const PayloadSharingData& payloadSharingData) noexcept;

[[maybe_unused]]
std::optional<PayloadSharingData> decodeWritePayloadSharing(const BLESensorConfiguration& config,const Data& data) noexcept;

[[maybe_unused]]
std::optional<Data> encodeImmediateSend(const BLESensorConfiguration& config,const ImmediateSendData& immediateSendData) noexcept;

[[maybe_unused]]
std::optional<ImmediateSendData> decodeImmediateSend(const BLESensorConfiguration& config,const Data& data) noexcept;

[[maybe_unused]]
SignalCharacteristicDataType detect(const BLESensorConfiguration& config,const Data& data) noexcept;

// THE FOLLOWING METHODS ARE MOVED TO THE CPP AND THUS HIDDEN FROM THE ABI
//byte signalDataActionCode(const byte[] signalData);

//bool int16(const byte[] data, const std::size_t index, int16& to); /// true if successful, sets to parameters

} // end namespace

} // end namespace
} // end namespace

#endif