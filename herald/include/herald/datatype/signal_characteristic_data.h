//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIGNAL_CHARACTERISTIC_DATA_H
#define SIGNAL_CHARACTERISTIC_DATA_H

#include "data.h"
#include "rssi.h"
#include "payload_data.h"
#include "payload_sharing_data.h"
#include "immediate_send_data.h"

#include <optional>

namespace herald {
namespace datatype {

enum class SignalCharacteristicDataType : short {
  rssi, payload, payloadSharing, immediateSend, unknown
};

namespace SignalCharacteristicData {

static std::optional<Data> encodeWriteRssi(const RSSI& rssi) noexcept;

static std::optional<RSSI> decodeWriteRSSI(const Data& data) noexcept;

static std::optional<Data> encodeWritePayload(const PayloadData& payloadData) noexcept;

static std::optional<PayloadData> decodeWritePayload(const Data& data) noexcept;

static std::optional<Data> encodeWritePayloadSharing(const PayloadSharingData& payloadSharingData) noexcept;

static std::optional<PayloadSharingData> decodeWritePayloadSharing(const Data& data) noexcept;

static std::optional<Data> encodeImmediateSend(const ImmediateSendData& immediateSendData) noexcept;

static std::optional<ImmediateSendData> decodeImmediateSend(const Data& data) noexcept;

static SignalCharacteristicDataType detect(const Data& data) noexcept;

// THE FOLLOWING METHODS ARE MOVED TO THE CPP AND THUS HIDDEN FROM THE ABI
//static byte signalDataActionCode(const byte[] signalData);

//static bool int16(const byte[] data, const std::size_t index, int16& to); /// true if successful, sets to parameters

} // end namespace

} // end namespace
} // end namespace

#endif