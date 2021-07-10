//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_device.h"
#include "herald/ble/ble_device_delegate.h"
#include "herald/ble/ble_tx_power.h"
#include "herald/ble/ble_mac_address.h"
#include "herald/ble/filter/ble_advert_parser.h"

#include "herald/datatype/date.h"
#include "herald/datatype/time_interval.h"
#include "herald/datatype/target_identifier.h"

#include <optional>
#include <algorithm>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;

BLEDevice::BLEDevice()
  : Device(),
    id(),
    delegate(std::nullopt),
    mCreated(Date()),
    lastUpdated(std::optional<Date>()),
    mState(std::optional<BLEDeviceState>(BLEDeviceState::uninitialised)),
    os(std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown)),
    payload(),
    mImmediateSendData(),
    mRssi(),
    mTxPower(),
    mReceiveOnly(false),
    mIgnore(false),
    ignoreForDuration(),
    ignoreUntil(), // empty, not "now"
    mPayloadCharacteristic(),
    mSignalCharacteristic(),
    pseudoAddress(),
    lastWriteRssiAt(),
    lastWritePayloadAt(),
    lastWritePayloadSharingAt(),
    lastDiscoveredAt(),
    connected(),
    payloadUpdated(),
    segments(),
    mServices(),
    hasEverConnected(false),
    connectRepeatedFailures(0)
{
  ;
}

BLEDevice::BLEDevice(TargetIdentifier identifier, BLEDeviceDelegate& del,
  const Date& createdAt)
  : Device(),
    id(identifier),
    delegate(std::optional(std::reference_wrapper<BLEDeviceDelegate>(del))),
    mCreated(createdAt),
    lastUpdated(std::optional<Date>()),
    mState(),
    os(std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown)),
    payload(),
    mImmediateSendData(),
    mRssi(),
    mTxPower(),
    mReceiveOnly(false),
    mIgnore(false),
    ignoreForDuration(),
    ignoreUntil(), // empty, not "now"
    mPayloadCharacteristic(),
    mSignalCharacteristic(),
    pseudoAddress(),
    lastWriteRssiAt(),
    lastWritePayloadAt(),
    lastWritePayloadSharingAt(),
    lastDiscoveredAt(),
    connected(),
    payloadUpdated(),
    segments(),
    mServices(),
    hasEverConnected(false),
    connectRepeatedFailures(0)
{
  ;
}

BLEDevice::BLEDevice(const BLEDevice& other)
  : Device(),
    id(other.id),
    delegate(other.delegate),
    mCreated(other.mCreated),
    lastUpdated(other.lastUpdated),
    mState(other.mState),
    os(other.os),
    payload(other.payload),
    mImmediateSendData(other.mImmediateSendData),
    mRssi(other.mRssi),
    mTxPower(other.mTxPower),
    mReceiveOnly(other.mReceiveOnly),
    mIgnore(other.mIgnore),
    ignoreForDuration(other.ignoreForDuration),
    ignoreUntil(other.ignoreUntil), // empty, not "now"
    mPayloadCharacteristic(other.mPayloadCharacteristic),
    mSignalCharacteristic(other.mSignalCharacteristic),
    pseudoAddress(other.pseudoAddress),
    lastWriteRssiAt(other.lastWriteRssiAt),
    lastWritePayloadAt(other.lastWritePayloadAt),
    lastWritePayloadSharingAt(other.lastWritePayloadSharingAt),
    lastDiscoveredAt(other.lastDiscoveredAt),
    connected(other.connected),
    payloadUpdated(other.payloadUpdated),
    segments(other.segments),
    mServices(other.mServices)
{
  ;
}

BLEDevice::~BLEDevice() = default;

void
BLEDevice::reset(const TargetIdentifier& newID, BLEDeviceDelegate& newDelegate)
{
  // mImpl = std::make_unique<Impl>();
  mState.reset();
  id = newID;
  delegate.emplace(std::reference_wrapper<BLEDeviceDelegate>(newDelegate));
}

BLEDevice&
BLEDevice::operator=(const BLEDevice& other)
{
  id = other.id;
  delegate = other.delegate;
  mCreated = other.mCreated;
  lastUpdated = other.lastUpdated;
  mState = other.mState;
  os = other.os;
  payload = other.payload;
  mImmediateSendData = other.mImmediateSendData;
  mRssi = other.mRssi;
  mTxPower = other.mTxPower;
  mReceiveOnly = other.mReceiveOnly;
  mIgnore = other.mIgnore;
  ignoreForDuration = other.ignoreForDuration;
  ignoreUntil = other.ignoreUntil; // empty, not "now"
  mPayloadCharacteristic = other.mPayloadCharacteristic;
  mSignalCharacteristic = other.mSignalCharacteristic;
  pseudoAddress = other.pseudoAddress;
  lastWriteRssiAt = other.lastWriteRssiAt;
  lastWritePayloadAt = other.lastWritePayloadAt;
  lastWritePayloadSharingAt = other.lastWritePayloadSharingAt;
  lastDiscoveredAt = other.lastDiscoveredAt;
  connected = other.connected;
  payloadUpdated = other.payloadUpdated;
  segments = other.segments;
  mServices = other.mServices;
  return *this;
}

bool
BLEDevice::operator==(const BLEDevice& other) const noexcept
{
  return id == other.id;
}

bool
BLEDevice::operator!=(const BLEDevice& other) const noexcept
{
  return id != other.id;
}


const TargetIdentifier&
BLEDevice::identifier() const
{
  return id;
}

void
BLEDevice::identifier(const TargetIdentifier& toCopyFrom)
{
  id = toCopyFrom;
}

Date
BLEDevice::created() const
{
  return mCreated;
}

// basic descriptors
std::string
BLEDevice::description() const
{
  return (std::string)id;
}

BLEDevice::operator std::string() const
{
  return (std::string)id;
}

// timing related getters
TimeInterval
BLEDevice::timeIntervalSinceLastUpdate() const
{
  if (!lastUpdated.has_value()) {
    return TimeInterval::zero(); // default to new, just seen rather than 'no activity'
  }
  return TimeInterval(lastUpdated.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceConnected() const
{
  if (mState != BLEDeviceState::connected) {
    return TimeInterval::never();
  }
  if (!connected.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(connected.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastPayloadDataUpdate() const
{
  if (!payloadUpdated.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(payloadUpdated.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayloadSharing() const
{
  if (!lastWritePayloadSharingAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(lastWritePayloadSharingAt.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayload() const
{
  if (!lastWritePayloadAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(lastWritePayloadAt.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWriteRssi() const
{
  if (!lastWriteRssiAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(lastWriteRssiAt.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalUntilIgnoreExpired() const
{
  if (!ignoreUntil.has_value()) {
    return TimeInterval::zero();
  }
  if (ignoreUntil.value() == TimeInterval::never()) {
    return TimeInterval::never();
  }
  return TimeInterval(Date(),ignoreUntil.value());
}

// property getters and setters
std::optional<BLEMacAddress>
BLEDevice::pseudoDeviceAddress() const
{
  return pseudoAddress;
}

void
BLEDevice::pseudoDeviceAddress(BLEMacAddress newAddress)
{
  if (!pseudoAddress.has_value() || pseudoAddress.value() != newAddress) {
    pseudoAddress.emplace(newAddress);
    lastUpdated.emplace(); // Constructs Date as now
  }
}

std::optional<BLEDeviceState>
BLEDevice::state() const
{
  return mState;
}

void
BLEDevice::state(BLEDeviceState newState)
{
  // Check if failed to connect
  if (BLEDeviceState::disconnected == newState &&
      (!mState.has_value() ||
       (mState.has_value() && BLEDeviceState::disconnected == mState ) ||
       (mState.has_value() && BLEDeviceState::connecting == mState )
      )
  ) {
    connectRepeatedFailures++;
    if (connectRepeatedFailures >= 5) { // Changed to 5 from 10 for quicker failure in busy areas
      // Ignore for a while (progressive backoff)
      operatingSystem(BLEDeviceOperatingSystem::ignore);
      // Don't backoff again immediately
      connectRepeatedFailures = 0;
    }
  }
  if (BLEDeviceState::connected == newState) {
    hasEverConnected = true;
    connectRepeatedFailures = 0;
  }
  bool changed = !mState.has_value() || mState.value() != newState;
  if (changed) {
    mState.emplace(newState);
    lastUpdated.emplace(); // Constructs Date as now
    delegate->get().device(*this, BLEDeviceAttribute::state);
  }
}

std::optional<BLEDeviceOperatingSystem>
BLEDevice::operatingSystem() const
{
  return os;
}

void
BLEDevice::operatingSystem(BLEDeviceOperatingSystem newOS)
{
  
  lastUpdated.emplace(); // Constructs Date as now
  if (os.has_value() && os == BLEDeviceOperatingSystem::ignore) {
    if (!ignoreForDuration.has_value()) {
      ignoreForDuration.emplace(TimeInterval::minutes(1));
    } else if (ignoreForDuration.value() < TimeInterval::minutes(3)) {
      // progressive backoff for unknown device
      ignoreForDuration.value() * 1.2;
      if (ignoreForDuration.value() > TimeInterval::minutes(7)) {
        // just ignore as the mac will have rotated (7:43 will occur half way through 15 mins intervals)
        // As the total BLE DB timeout is ~ 25 minutes, this will save significant connection attempt cycles
        mIgnore = true;
      }
    }
    ignoreUntil.emplace(lastUpdated.value() + ignoreForDuration.value());
  } else {
    ignoreUntil.reset();
  }
  if (os == BLEDeviceOperatingSystem::ios || os == BLEDeviceOperatingSystem::android) {
    ignoreForDuration.reset();
  }
  bool changed = !os.has_value() || os.value() != newOS;
  if (changed) {
    os.emplace(newOS);
    delegate->get().device(*this, BLEDeviceAttribute::operatingSystem);
  }
}

std::optional<PayloadData>
BLEDevice::payloadData() const
{
  return payload;
}

void
BLEDevice::payloadData(PayloadData newPayloadData)
{
  bool changed = !payload.has_value() || payload.value() != newPayloadData;
  if (changed) {
    payload.emplace(newPayloadData);
    lastUpdated.emplace(); // Constructs Date as now
    payloadUpdated.emplace();
    if (delegate.has_value()) {
      delegate->get().device(*this, BLEDeviceAttribute::payloadData);
    }
  }
}

std::optional<ImmediateSendData>
BLEDevice::immediateSendData() const
{
  return mImmediateSendData;
}

void
BLEDevice::immediateSendData(ImmediateSendData toSend)
{
  bool changed = !mImmediateSendData.has_value() || mImmediateSendData.value() != toSend;
  if (changed) {
    mImmediateSendData.emplace(toSend);
    lastUpdated.emplace(); // Constructs Date as now
    delegate->get().device(*this, BLEDeviceAttribute::immediateSendData);
  }
}

void
BLEDevice::clearImmediateSendData()
{
  mImmediateSendData.reset();
}

std::optional<RSSI>
BLEDevice::rssi() const
{
  return mRssi;
}

void
BLEDevice::rssi(RSSI newRSSI)
{
  bool changed = !mRssi.has_value() || mRssi.value() != newRSSI;
  if (changed) {
    mRssi.emplace(newRSSI);
    lastUpdated.emplace(); // Constructs Date as now
    if (delegate.has_value()) {
      delegate->get().device(*this, BLEDeviceAttribute::rssi);
    }
  }
}

std::optional<BLETxPower>
BLEDevice::txPower() const
{
  return mTxPower;
}

void
BLEDevice::txPower(BLETxPower newPower)
{
  bool changed = !mTxPower.has_value() || mTxPower.value() != newPower;
  if (changed) {
    mTxPower.emplace(newPower);
    lastUpdated.emplace(); // Constructs Date as now
    delegate->get().device(*this, BLEDeviceAttribute::txPower);
  }
}

bool
BLEDevice::receiveOnly() const
{
  return mReceiveOnly;
}

void
BLEDevice::receiveOnly(bool newReceiveOnly)
{
  mReceiveOnly = newReceiveOnly;
}

std::optional<UUID>
BLEDevice::signalCharacteristic() const
{
  return mSignalCharacteristic;
}

void
BLEDevice::signalCharacteristic(UUID newChar)
{
  mSignalCharacteristic.emplace(newChar);
}

std::optional<UUID>
BLEDevice::payloadCharacteristic() const
{
  return mPayloadCharacteristic;
}

void
BLEDevice::payloadCharacteristic(UUID newChar)
{
  mPayloadCharacteristic.emplace(newChar);
}

// State engine methods
bool
BLEDevice::ignore() const
{
  // Check for permanent ignore
  if (mIgnore) {
    return true;
  }
  // Check for timed ignore
  if (!ignoreUntil.has_value()) {
    return false;
  }
  if (Date() < ignoreUntil.value()) {
    return true;
  }
  return false;
}

void
BLEDevice::ignore(bool newIgnore) // set permanent ignore flag
{
  mIgnore = newIgnore;
}

void
BLEDevice::invalidateCharacteristics()
{
  mPayloadCharacteristic.reset();
  mSignalCharacteristic.reset();
}

void
BLEDevice::registerDiscovery(Date at)
{
  lastDiscoveredAt.emplace(at);
  lastUpdated.reset(); // TODO verify this shouldn't be 'at' instead (for continuity)
}

void
BLEDevice::registerWritePayload(Date at)
{
  lastUpdated.emplace(at);
  lastWritePayloadAt.emplace(at);
}

void
BLEDevice::registerWritePayloadSharing(Date at)
{
  lastUpdated.emplace(at);
  lastWritePayloadSharingAt.emplace(at);
}

void
BLEDevice::registerWriteRssi(Date at)
{
  lastUpdated.emplace(at);
  lastWriteRssiAt.emplace(at);
}

bool
BLEDevice::hasAdvertData() const
{
  return segments.has_value();
}

void
BLEDevice::advertData(std::vector<BLEAdvertSegment> newSegments)
{
  segments = newSegments;
}

const std::vector<BLEAdvertSegment>&
BLEDevice::advertData() const
{
  return segments.value();
}

bool
BLEDevice::hasServicesSet() const
{
  return mServices.has_value();
}
void
BLEDevice::services(std::vector<UUID> services)
{
  lastUpdated.emplace(); // Constructs Date as now
  mServices.emplace(services);
}

bool
BLEDevice::hasService(const UUID& serviceUUID) const
{
  if (!mServices.has_value()) return false; // guard
  auto iter = std::find(std::begin(mServices.value()),std::end(mServices.value()),serviceUUID);
  return (std::end(mServices.value()) != iter);
}

}
}