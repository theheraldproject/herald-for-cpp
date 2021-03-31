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

class BLEDevice::Impl {
public:
  Impl(TargetIdentifier identifier, BLEDeviceDelegate& del, const Date& createdAt);
  Impl(const Impl& other); // copy ctor
  Impl(Impl&& other) = delete;
  ~Impl();

  Impl& operator=(const Impl& other); // copy assign
  Impl operator=(Impl&& other) = delete;

  TargetIdentifier id;
  BLEDeviceDelegate& delegate;

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
  std::optional<TimeInterval> ignoreForDuration;
  std::optional<Date> ignoreUntil;

  std::optional<UUID> payloadCharacteristic;
  std::optional<UUID> signalCharacteristic;
  std::optional<BLEMacAddress> pseudoAddress;

  std::optional<Date> lastWriteRssiAt;
  std::optional<Date> lastWritePayloadAt;
  std::optional<Date> lastWritePayloadSharingAt;
  std::optional<Date> lastDiscoveredAt;
  std::optional<Date> connected;
  std::optional<Date> payloadUpdated;

  std::optional<std::vector<BLEAdvertSegment>> segments;
  std::optional<std::vector<UUID>> services;

  bool hasEverConnected;
  int connectRepeatedFailures;
};

BLEDevice::Impl::Impl(TargetIdentifier identifier, BLEDeviceDelegate& del, const Date& createdAt)
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
    ignoreForDuration(),
    ignoreUntil(), // empty, not "now"
    payloadCharacteristic(),
    signalCharacteristic(),
    pseudoAddress(),
    lastWriteRssiAt(),
    lastWritePayloadAt(),
    lastWritePayloadSharingAt(),
    lastDiscoveredAt(),
    connected(),
    payloadUpdated(),
    segments(),
    services(),
    hasEverConnected(false),
    connectRepeatedFailures(0)
{
  ;
}

BLEDevice::Impl::Impl(const Impl& other)
  : id(other.id),
    delegate(other.delegate),
    created(other.created),
    lastUpdated(other.lastUpdated),
    state(other.state),
    os(other.os),
    payload(other.payload),
    immediateSendData(other.immediateSendData),
    rssi(other.rssi),
    txPower(other.txPower),
    receiveOnly(other.receiveOnly),
    ignore(other.ignore),
    ignoreForDuration(other.ignoreForDuration),
    ignoreUntil(other.ignoreUntil), // empty, not "now"
    payloadCharacteristic(other.payloadCharacteristic),
    signalCharacteristic(other.signalCharacteristic),
    pseudoAddress(other.pseudoAddress),
    lastWriteRssiAt(other.lastWriteRssiAt),
    lastWritePayloadAt(other.lastWritePayloadAt),
    lastWritePayloadSharingAt(other.lastWritePayloadSharingAt),
    lastDiscoveredAt(other.lastDiscoveredAt),
    connected(other.connected),
    payloadUpdated(other.payloadUpdated),
    segments(other.segments),
    services(other.services)
{
  ;
}

BLEDevice::Impl::~Impl()
{
  ;
}

BLEDevice::Impl&
BLEDevice::Impl::operator=(const Impl& other)
{
  id = other.id;
  delegate = other.delegate;
  created = other.created;
  lastUpdated = other.lastUpdated;
  state = other.state;
  os = other.os;
  payload = other.payload;
  immediateSendData = other.immediateSendData;
  rssi = other.rssi;
  txPower = other.txPower;
  receiveOnly = other.receiveOnly;
  ignore = other.ignore;
  ignoreForDuration = other.ignoreForDuration;
  ignoreUntil = other.ignoreUntil;
  payloadCharacteristic = other.payloadCharacteristic;
  signalCharacteristic = other.signalCharacteristic;
  pseudoAddress = other.pseudoAddress;
  lastWriteRssiAt = other.lastWriteRssiAt;
  lastWritePayloadAt = other.lastWritePayloadAt;
  lastWritePayloadSharingAt = other.lastWritePayloadSharingAt;
  lastDiscoveredAt = other.lastDiscoveredAt;
  connected = other.connected;
  payloadUpdated = other.payloadUpdated;
  segments = other.segments;
  services = other.services;

  return *this;
}





BLEDevice::BLEDevice(TargetIdentifier identifier, BLEDeviceDelegate& delegate,
  const Date& createdAt)
  : Device(),
    mImpl(std::make_unique<Impl>(identifier,delegate,createdAt))
{
  ;
}

BLEDevice::BLEDevice(const BLEDevice& other)
  : Device(),
    mImpl(std::make_unique<Impl>(*other.mImpl))
{
  ;
}

BLEDevice::~BLEDevice()
{
  ;
}

BLEDevice&
BLEDevice::operator=(const BLEDevice& other)
{
  mImpl = std::make_unique<Impl>(*other.mImpl);
  return *this;
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
    return TimeInterval::zero(); // default to new, just seen rather than 'no activity'
  }
  return TimeInterval(mImpl->lastUpdated.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceConnected() const
{
  if (state() != BLEDeviceState::connected) {
    return TimeInterval::never();
  }
  if (!mImpl->connected.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(mImpl->connected.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastPayloadDataUpdate() const
{
  if (!mImpl->payloadUpdated.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(mImpl->payloadUpdated.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayloadSharing() const
{
  if (!mImpl->lastWritePayloadSharingAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(mImpl->lastWritePayloadSharingAt.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWritePayload() const
{
  if (!mImpl->lastWritePayloadAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(mImpl->lastWritePayloadAt.value(), Date());
}

TimeInterval
BLEDevice::timeIntervalSinceLastWriteRssi() const
{
  if (!mImpl->lastWriteRssiAt.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(mImpl->lastWriteRssiAt.value(),Date());
}

TimeInterval
BLEDevice::timeIntervalUntilIgnoreExpired() const
{
  if (!mImpl->ignoreUntil.has_value()) {
    return TimeInterval::zero();
  }
  if (mImpl->ignoreUntil.value() == TimeInterval::never()) {
    return TimeInterval::never();
  }
  return TimeInterval(Date(),mImpl->ignoreUntil.value());
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
  if (!mImpl->pseudoAddress.has_value() || mImpl->pseudoAddress.value() != newAddress) {
    mImpl->pseudoAddress = newAddress;
    mImpl->lastUpdated.emplace(); // Constructs Date as now
  }
}

std::optional<BLEDeviceState>
BLEDevice::state() const
{
  return mImpl->state;
}

void
BLEDevice::state(BLEDeviceState newState)
{
  // Check if failed to connect
  if (BLEDeviceState::disconnected == newState &&
      (!mImpl->state.has_value() ||
       (mImpl->state.has_value() && BLEDeviceState::disconnected == mImpl->state ) ||
       (mImpl->state.has_value() && BLEDeviceState::connecting == mImpl->state )
      )
  ) {
    mImpl->connectRepeatedFailures++;
    if (mImpl->connectRepeatedFailures >= 5) { // Changed to 5 from 10 for quicker failure in busy areas
      // Ignore for a while (progressive backoff)
      operatingSystem(BLEDeviceOperatingSystem::ignore);
      // Don't backoff again immediately
      mImpl->connectRepeatedFailures = 0;
    }
  }
  if (BLEDeviceState::connected == newState) {
    mImpl->hasEverConnected = true;
    mImpl->connectRepeatedFailures = 0;
  }
  bool changed = !mImpl->state.has_value() || mImpl->state.value() != newState;
  if (changed) {
    mImpl->state.emplace(newState);
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::state);
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
  
  mImpl->lastUpdated.emplace(); // Constructs Date as now
  if (mImpl->os.has_value() && mImpl->os == BLEDeviceOperatingSystem::ignore) {
    if (!mImpl->ignoreForDuration.has_value()) {
      mImpl->ignoreForDuration = TimeInterval::minutes(1);
    } else if (mImpl->ignoreForDuration.value() < TimeInterval::minutes(3)) {
      // progressive backoff for unknown device
      mImpl->ignoreForDuration.value() * 1.2;
      if (mImpl->ignoreForDuration.value() > TimeInterval::minutes(7)) {
        // just ignore as the mac will have rotated (7:43 will occur half way through 15 mins intervals)
        // As the total BLE DB timeout is ~ 25 minutes, this will save significant connection attempt cycles
        mImpl->ignore = true;
      }
    }
    mImpl->ignoreUntil = mImpl->lastUpdated.value() + mImpl->ignoreForDuration.value();
  } else {
    mImpl->ignoreUntil.reset();
  }
  if (mImpl->os == BLEDeviceOperatingSystem::ios || mImpl->os == BLEDeviceOperatingSystem::android) {
    mImpl->ignoreForDuration.reset();
  }
  bool changed = !mImpl->os.has_value() || mImpl->os.value() != newOS;
  if (changed) {
    mImpl->os.emplace(newOS);
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::operatingSystem);
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
  if (changed) {
    mImpl->payload.emplace(newPayloadData);
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->payloadUpdated = Date();
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::payloadData);
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
  if (changed) {
    mImpl->immediateSendData.emplace(toSend);
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::immediateSendData);
  }
}

void
BLEDevice::clearImmediateSendData()
{
  mImpl->immediateSendData.reset();
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
  if (changed) {
    mImpl->rssi.emplace(newRSSI);
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::rssi);
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
  if (changed) {
    mImpl->txPower.emplace(newPower);
    mImpl->lastUpdated.emplace(); // Constructs Date as now
    mImpl->delegate.device(shared_from_this(), BLEDeviceAttribute::txPower);
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
  // Check for permanent ignore
  if (mImpl->ignore) {
    return true;
  }
  // Check for timed ignore
  if (!mImpl->ignoreUntil.has_value()) {
    return false;
  }
  if (Date() < mImpl->ignoreUntil.value()) {
    return true;
  }
  return false;
}

void
BLEDevice::ignore(bool newIgnore) // set permanent ignore flag
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
BLEDevice::registerDiscovery(Date at)
{
  mImpl->lastDiscoveredAt = at;
}

void
BLEDevice::registerWritePayload(Date at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWritePayloadAt = at;
}

void
BLEDevice::registerWritePayloadSharing(Date at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWritePayloadSharingAt = at;
}

void
BLEDevice::registerWriteRssi(Date at)
{
  mImpl->lastUpdated = at;
  mImpl->lastWriteRssiAt = at;
}

bool
BLEDevice::hasAdvertData() const
{
  return mImpl->segments.has_value();
}

void
BLEDevice::advertData(std::vector<BLEAdvertSegment> segments)
{
  mImpl->segments = segments;
}

std::vector<BLEAdvertSegment>&
BLEDevice::advertData() const
{
  return mImpl->segments.value();
}

bool
BLEDevice::hasServicesSet() const
{
  return mImpl->services.has_value();
}
void
BLEDevice::services(std::vector<UUID> services)
{
  mImpl->lastUpdated.emplace(); // Constructs Date as now
  mImpl->services = services;
}

bool
BLEDevice::hasService(const UUID& serviceUUID) const
{
  if (!mImpl->services.has_value()) return false; // guard
  auto iter = std::find(std::begin(mImpl->services.value()),std::end(mImpl->services.value()),serviceUUID);
  return (std::end(mImpl->services.value()) != iter);
}

}
}