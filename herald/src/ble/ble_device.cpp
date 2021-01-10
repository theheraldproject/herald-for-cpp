//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_device.h"
#include "herald/ble/ble_device_delegate.h"
#include "herald/ble/ble_tx_power.h"
#include "herald/ble/ble_mac_address.h"

#include "herald/datatype/date.h"
#include "herald/datatype/time_interval.h"
#include "herald/datatype/target_identifier.h"

#include <optional>

namespace herald {
namespace ble {

class BLEDevice::Impl {
public:
  Impl(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> del, const Date& createdAt);
  ~Impl();

  TargetIdentifier id;
  std::shared_ptr<BLEDeviceDelegate> delegate;

  // Data holders
  Date created;
  std::optional<Date> lastUpdated;

  std::optional<BLEDeviceState> state;
  std::optional<BLEDeviceOperatingSystem> os;
  std::optional<PayloadData> payload;
  std::optional<ImmediateSendData> immediateSendData;
  std::optional<RSSI> rssi;
  std::optional<BLETxPower> txPower;
  bool receiveOnly;
  bool ignore;

  std::optional<UUID> payloadCharacteristic;
  std::optional<UUID> signalCharacteristic;
  std::optional<BLEMacAddress> pseudoAddress;

  std::optional<Date> lastWriteRssiAt;
  std::optional<Date> lastWritePayloadAt;
  std::optional<Date> lastWritePayloadSharingAt;
  std::optional<Date> lastDiscoveredAt;
  std::optional<Date> connected;
  std::optional<Date> payloadUpdated;
};

BLEDevice::Impl::Impl(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> del, const Date& createdAt)
  : id(identifier),
    delegate(del),
    created(createdAt),
    lastUpdated(std::optional<Date>()),
    state(),
    os(std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown)),
    payload(),
    immediateSendData(),
    rssi(),
    txPower(),
    receiveOnly(false),
    ignore(false),
    payloadCharacteristic(),
    signalCharacteristic(),
    pseudoAddress(),
    lastWriteRssiAt(),
    lastWritePayloadAt(),
    lastWritePayloadSharingAt(),
    lastDiscoveredAt(),
    connected(),
    payloadUpdated()
{
  ;
}

BLEDevice::Impl::~Impl()
{
  ;
}





BLEDevice::BLEDevice(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> delegate,
  const Date& createdAt)
  : Device(),
    mImpl(std::make_unique<Impl>(identifier,delegate,createdAt))
{
  ;
}

BLEDevice::~BLEDevice()
{
  ;
}


const TargetIdentifier&
BLEDevice::identifier() const
{
  return mImpl->id;
}

Date
BLEDevice::created() const
{
  return mImpl->created;
}

// basic descriptors
std::string
BLEDevice::description() const
{
  return (std::string)mImpl->id;
}

BLEDevice::operator std::string() const
{
  return (std::string)mImpl->id;
}

// timing related getters
TimeInterval
BLEDevice::timeIntervalSinceLastUpdate() const
{
  if (!mImpl->lastUpdated.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->lastUpdated.value();
}

TimeInterval
BLEDevice::timeIntervalSinceConnected() const
{
  if (!mImpl->connected.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->connected.value();
}

TimeInterval
BLEDevice::timeIntervalSinceLastPayloadDataUpdate() const
{
  if (!mImpl->payloadUpdated.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->payloadUpdated.value();
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayloadSharing() const
{
  if (!mImpl->lastWritePayloadSharingAt.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->lastWritePayloadSharingAt.value();
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayload() const
{
  if (!mImpl->lastWritePayloadAt.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->lastWritePayloadAt.value();
}

TimeInterval
BLEDevice::timeIntervalSinceLastWriteRssi() const
{
  if (!mImpl->lastWriteRssiAt.has_value()) {
    return TimeInterval::never();
  }
  return Date() - mImpl->lastWriteRssiAt.value();
}

// property getters and setters
std::optional<BLEMacAddress>
BLEDevice::pseudoDeviceAddress() const
{
  return mImpl->pseudoAddress;
}

void
BLEDevice::pseudoDeviceAddress(BLEMacAddress newAddress)
{
  mImpl->pseudoAddress = newAddress;
}

std::optional<BLEDeviceState>
BLEDevice::state() const
{
  return mImpl->state;
}

void
BLEDevice::state(BLEDeviceState newState)
{
  bool changed = !mImpl->state.has_value() || mImpl->state.value() != newState;
  mImpl->state.emplace(newState);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::state);
  }
}

std::optional<BLEDeviceOperatingSystem>
BLEDevice::operatingSystem() const
{
  return mImpl->os;
}

void
BLEDevice::operatingSystem(BLEDeviceOperatingSystem newOS)
{
  bool changed = !mImpl->os.has_value() || mImpl->os.value() != newOS;
  mImpl->os.emplace(newOS);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::operatingSystem);
  }
}

std::optional<PayloadData>
BLEDevice::payloadData() const
{
  return mImpl->payload;
}

void
BLEDevice::payloadData(PayloadData newPayloadData)
{
  bool changed = !mImpl->payload.has_value() || mImpl->payload.value() != newPayloadData;
  mImpl->payload.emplace(newPayloadData);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->payloadUpdated = Date();
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::payloadData);
  }
}

std::optional<ImmediateSendData>
BLEDevice::immediateSendData() const
{
  return mImpl->immediateSendData;
}

void
BLEDevice::immediateSendData(ImmediateSendData toSend)
{
  bool changed = !mImpl->immediateSendData.has_value() || mImpl->immediateSendData.value() != toSend;
  mImpl->immediateSendData.emplace(toSend);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::immediateSendData);
  }
}

std::optional<RSSI>
BLEDevice::rssi() const
{
  return mImpl->rssi;
}
void
BLEDevice::rssi(RSSI newRSSI)
{
  bool changed = !mImpl->rssi.has_value() || mImpl->rssi.value() != newRSSI;
  mImpl->rssi.emplace(newRSSI);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::rssi);
  }
}

std::optional<BLETxPower>
BLEDevice::txPower() const
{
  return mImpl->txPower;
}

void
BLEDevice::txPower(BLETxPower newPower)
{
  bool changed = !mImpl->txPower.has_value() || mImpl->txPower.value() != newPower;
  mImpl->txPower.emplace(newPower);
  if (changed) {
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate->device(shared_from_this(), BLEDeviceAttribute::txPower);
  }
}

bool
BLEDevice::receiveOnly() const
{
  return mImpl->receiveOnly;
}

void
BLEDevice::receiveOnly(bool newReceiveOnly)
{
  mImpl->receiveOnly = newReceiveOnly;
}

std::optional<UUID>
BLEDevice::signalCharacteristic() const
{
  return mImpl->signalCharacteristic;
}

void
BLEDevice::signalCharacteristic(UUID newChar)
{
  mImpl->signalCharacteristic = newChar;
}

std::optional<UUID>
BLEDevice::payloadCharacteristic() const
{
  return mImpl->payloadCharacteristic;
}

void
BLEDevice::payloadCharacteristic(UUID newChar)
{
  mImpl->payloadCharacteristic = newChar;
}

// State engine methods
bool
BLEDevice::ignore() const
{
  return mImpl->ignore;
}

void
BLEDevice::ignore(bool newIgnore)
{
  mImpl->ignore = newIgnore;
}

void
BLEDevice::invalidateCharacteristics()
{
  mImpl->payloadCharacteristic.reset();
  mImpl->signalCharacteristic.reset();
}

void
BLEDevice::registerDiscovery(Date& at)
{
  mImpl->lastDiscoveredAt = at;
}

void
BLEDevice::registerWritePayload(Date& at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWritePayloadAt = at;
}

void
BLEDevice::registerWritePayloadSharing(Date& at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWritePayloadSharingAt = at;
}

void
BLEDevice::registerWriteRssi(Date& at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWriteRssiAt = at;
}


}
}