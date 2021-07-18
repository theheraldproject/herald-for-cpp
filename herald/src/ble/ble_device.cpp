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

BLEDeviceFlags::BLEDeviceFlags() : bitFields(64000) /* TTT TT FTF FFFFF ?(F)?(F) F */
{
  ;
}

void
BLEDeviceFlags::reset()
{
  bitFields.reset();

  bitFields.set(0);
  bitFields.set(1);
  bitFields.set(2);

  bitFields.set(3);
  bitFields.set(4);

  bitFields.set(6);
}

BLEInternalState
BLEDeviceFlags::internalState() const
{
  if (bitFields.test(0)) {
    if (bitFields.test(1)) {
      if (bitFields.test(2)) {
        return BLEInternalState::discovered;
      } else {
        return BLEInternalState::filtered;
      }
    } else {
      if (bitFields.test(2)) {
        return BLEInternalState::has_potential;
      } else {
        return BLEInternalState::relevant;
      }
    }
  } else {
    if (bitFields.test(1)) {
      if (bitFields.test(2)) {
        return BLEInternalState::identified;
      } else {
        return BLEInternalState::timed_out;
      }
    }
    // remaining values reserved
  }
}

void
BLEDeviceFlags::internalState(const BLEInternalState newState)
{
  switch (newState)
  {
  case BLEInternalState::discovered:
    bitFields.set(0,true);
    bitFields.set(1,true);
    bitFields.set(2,true);
    break;
  case BLEInternalState::filtered:
    bitFields.set(0,true);
    bitFields.set(1,true);
    bitFields.set(2,false);
    break;
  case BLEInternalState::has_potential:
    bitFields.set(0,true);
    bitFields.set(1,false);
    bitFields.set(2,true);
    break;
  case BLEInternalState::relevant:
    bitFields.set(0,true);
    bitFields.set(1,false);
    bitFields.set(2,false);
    break;
  case BLEInternalState::identified:
    bitFields.set(0,false);
    bitFields.set(1,true);
    bitFields.set(2,true);
    break;
  case BLEInternalState::timed_out:
    bitFields.set(0,false);
    bitFields.set(1,true);
    bitFields.set(2,false);
    break;
  }
}

std::optional<BLEDeviceState>
BLEDeviceFlags::state() const
{
  if (bitFields.test(3)) {
    if (bitFields.test(4)) {
      return std::optional({BLEDeviceState::uninitialised});
    } else {
      return std::optional({BLEDeviceState::connecting});
    }
  } else {
    if (bitFields.test(4)) {
      return std::optional({BLEDeviceState::connected});
    } else {
      return std::optional({BLEDeviceState::disconnected});
    }
  }
  return {};
}

void
BLEDeviceFlags::state(const BLEDeviceState newState)
{
  switch (newState)
  {
  case BLEDeviceState::uninitialised:
    bitFields.set(3,true);
    bitFields.set(4,true);
    break;
  case BLEDeviceState::connecting:
    bitFields.set(3,true);
    bitFields.set(4,false);
    break;
  case BLEDeviceState::connected:
    bitFields.set(3,false);
    bitFields.set(4,true);
    break;
  case BLEDeviceState::disconnected:
    bitFields.set(3,false);
    bitFields.set(4,false);
    break;
  }
}


std::optional<BLEDeviceOperatingSystem>
BLEDeviceFlags::operatingSystem() const
{
  if (bitFields.test(0) && bitFields.test(1) && !bitFields.test(2)) {
    return BLEDeviceOperatingSystem::ignore;
  }
  if (bitFields.test(5)) {
    if (bitFields.test(6)) {
      if (bitFields.test(7)) {
        return BLEDeviceOperatingSystem::android_tbc;
      } else {
        return BLEDeviceOperatingSystem::android;
      }
    } else {
      if (bitFields.test(7)) {
        return BLEDeviceOperatingSystem::ios_tbc;
      } else {
        return BLEDeviceOperatingSystem::ios;
      }
    }
  } else {
    if (bitFields.test(6)) {
      if (bitFields.test(7)) {
        return BLEDeviceOperatingSystem::shared;
      } else {
        return BLEDeviceOperatingSystem::unknown;
      }
    }
    // remaining values reserved
  }
  return {};
}

void
BLEDeviceFlags::operatingSystem(BLEDeviceOperatingSystem newOS)
{
  switch (newOS)
  {
  case BLEDeviceOperatingSystem::android_tbc:
    bitFields.set(5,true);
    bitFields.set(6,true);
    bitFields.set(7,true);
    break;
  case BLEDeviceOperatingSystem::android:
    bitFields.set(5,true);
    bitFields.set(6,true);
    bitFields.set(7,false);
    break;
  case BLEDeviceOperatingSystem::ios_tbc:
    bitFields.set(5,true);
    bitFields.set(6,false);
    bitFields.set(7,true);
    break;
  case BLEDeviceOperatingSystem::ios:
    bitFields.set(5,true);
    bitFields.set(6,false);
    bitFields.set(7,false);
    break;
  case BLEDeviceOperatingSystem::shared:
    bitFields.set(5,false);
    bitFields.set(6,true);
    bitFields.set(7,true);
    break;
  case BLEDeviceOperatingSystem::unknown:
    bitFields.set(5,false);
    bitFields.set(6,true);
    bitFields.set(7,false);
    break;
  case BLEDeviceOperatingSystem::ignore:
    // This is now tracked in the Filtered internal state field instead
    bitFields.set(0,true);
    bitFields.set(1,true);
    bitFields.set(2,false);
  }
}


bool
BLEDeviceFlags::hasHeraldService() const
{
  return bitFields.test(8);
}

void
BLEDeviceFlags::hasHeraldService(bool newValue)
{
  bitFields.set(8,newValue);
}

bool
BLEDeviceFlags::hasLegacyService() const
{
  return bitFields.test(9);
}

void
BLEDeviceFlags::hasLegacyService(bool newValue)
{
  bitFields.set(9,newValue);
}

bool
BLEDeviceFlags::hasPayloadCharacteristic() const
{
  return bitFields.test(10);
}

void
BLEDeviceFlags::hasPayloadCharacteristic(bool newValue)
{
  bitFields.set(10,newValue);
}

bool
BLEDeviceFlags::hasSignalCharacteristic() const
{
  return bitFields.test(11);
}

void
BLEDeviceFlags::hasSignalCharacteristic(bool newValue)
{
  bitFields.set(11,newValue);
}

bool
BLEDeviceFlags::hasSecureCharacteristic() const
{
  return bitFields.test(12);
}

void
BLEDeviceFlags::hasSecureCharacteristic(bool newValue)
{
  bitFields.set(12,newValue);
}

bool
BLEDeviceFlags::hasEverConnected() const
{
  return bitFields.test(15);
}

void
BLEDeviceFlags::hasEverConnected(bool newValue)
{
  bitFields.set(15,newValue);
}





BLEDevice::BLEDevice()
  : Device(),
    delegate(std::nullopt),
    id(),
    flags(),
    lastUpdated(Date(0)),
    stateData(std::monostate())
    // mCreated(Date()),
    // lastUpdated(std::optional<Date>()),
    // mState(std::optional<BLEDeviceState>(BLEDeviceState::uninitialised)),
    // os(std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown)),
    // payload(),
    // mImmediateSendData(),
    // mRssi(),
    // mTxPower(),
    // mReceiveOnly(false),
    // mIgnore(false),
    // ignoreForDuration(),
    // ignoreUntil(), // empty, not "now"
    // mPayloadCharacteristic(),
    // mSignalCharacteristic(),
    // pseudoAddress(),
    // lastWriteRssiAt(),
    // lastWritePayloadAt(),
    // lastWritePayloadSharingAt(),
    // lastDiscoveredAt(),
    // connected(),
    // payloadUpdated(),
    // segments(),
    // mServices(),
    // hasEverConnected(false),
    // connectRepeatedFailures(0)
{
  ;
}

BLEDevice::BLEDevice(TargetIdentifier identifier, BLEDeviceDelegate& del,
  const Date& createdAt)
  : Device(),
    delegate(std::optional(std::reference_wrapper<BLEDeviceDelegate>(del))),
    id(identifier),
    flags(),
    lastUpdated(createdAt),
    stateData(DiscoveredState())
    // mCreated(createdAt),
    // lastUpdated(std::optional<Date>()),
    // mState(),
    // os(std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown)),
    // payload(),
    // mImmediateSendData(),
    // mRssi(),
    // mTxPower(),
    // mReceiveOnly(false),
    // mIgnore(false),
    // ignoreForDuration(),
    // ignoreUntil(), // empty, not "now"
    // mPayloadCharacteristic(),
    // mSignalCharacteristic(),
    // pseudoAddress(),
    // lastWriteRssiAt(),
    // lastWritePayloadAt(),
    // lastWritePayloadSharingAt(),
    // lastDiscoveredAt(),
    // connected(),
    // payloadUpdated(),
    // segments(),
    // mServices(),
    // hasEverConnected(false),
    // connectRepeatedFailures(0)
{
  ;
}

BLEDevice::BLEDevice(const BLEDevice& other)
  : Device(),
    delegate(other.delegate),
    id(other.id),
    flags(other.flags),
    lastUpdated(other.lastUpdated),
    stateData(other.stateData)
    // mCreated(other.mCreated),
    // mState(other.mState),
    // os(other.os),
    // payload(other.payload),
    // mImmediateSendData(other.mImmediateSendData),
    // mRssi(other.mRssi),
    // mTxPower(other.mTxPower),
    // mReceiveOnly(other.mReceiveOnly),
    // mIgnore(other.mIgnore),
    // ignoreForDuration(other.ignoreForDuration),
    // ignoreUntil(other.ignoreUntil), // empty, not "now"
    // mPayloadCharacteristic(other.mPayloadCharacteristic),
    // mSignalCharacteristic(other.mSignalCharacteristic),
    // pseudoAddress(other.pseudoAddress),
    // lastWriteRssiAt(other.lastWriteRssiAt),
    // lastWritePayloadAt(other.lastWritePayloadAt),
    // lastWritePayloadSharingAt(other.lastWritePayloadSharingAt),
    // lastDiscoveredAt(other.lastDiscoveredAt),
    // connected(other.connected),
    // payloadUpdated(other.payloadUpdated),
    // segments(other.segments),
    // mServices(other.mServices)
{
  ;
}

BLEDevice::~BLEDevice() = default;

void
BLEDevice::reset(const TargetIdentifier& newID, BLEDeviceDelegate& newDelegate)
{
  delegate.emplace(std::reference_wrapper<BLEDeviceDelegate>(newDelegate));
  id = newID;
  // mImpl = std::make_unique<Impl>();
  // mState.reset();
  stateData = std::monostate();
}

BLEDevice&
BLEDevice::operator=(const BLEDevice& other)
{
  delegate = other.delegate;
  id = other.id;
  flags = other.flags;
  // mCreated = other.mCreated;
  lastUpdated = other.lastUpdated;
  stateData = other.stateData;
  // mState = other.mState;
  // os = other.os;
  // payload = other.payload;
  // mImmediateSendData = other.mImmediateSendData;
  // mRssi = other.mRssi;
  // mTxPower = other.mTxPower;
  // mReceiveOnly = other.mReceiveOnly;
  // mIgnore = other.mIgnore;
  // ignoreForDuration = other.ignoreForDuration;
  // ignoreUntil = other.ignoreUntil; // empty, not "now"
  // mPayloadCharacteristic = other.mPayloadCharacteristic;
  // mSignalCharacteristic = other.mSignalCharacteristic;
  // pseudoAddress = other.pseudoAddress;
  // lastWriteRssiAt = other.lastWriteRssiAt;
  // lastWritePayloadAt = other.lastWritePayloadAt;
  // lastWritePayloadSharingAt = other.lastWritePayloadSharingAt;
  // lastDiscoveredAt = other.lastDiscoveredAt;
  // connected = other.connected;
  // payloadUpdated = other.payloadUpdated;
  // segments = other.segments;
  // mServices = other.mServices;
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

// Date
// BLEDevice::created() const
// {
//   return mCreated;
// }

// basic descriptors
// std::string
// BLEDevice::description() const
// {
//   return (std::string)id;
// }

BLEDevice::operator std::string() const
{
  return (std::string)id;
}

// timing related getters
TimeInterval
BLEDevice::timeIntervalSinceLastUpdate() const
{
  // if (std::monostate == stateData) {
  //   return TimeInterval::zero();
  // }
  return lastUpdated;
  // if (!lastUpdated.has_value()) {
  //   return TimeInterval::zero(); // default to new, just seen rather than 'no activity'
  // }
  // return TimeInterval(lastUpdated.value(), Date());
}

// TimeInterval
// BLEDevice::timeIntervalSinceConnected() const
// {
//   if (flags.state() != BLEDeviceState::connected) {
//     return TimeInterval::never();
//   }
//   // if (!connected.has_value()) {
//   //   return TimeInterval::never();
//   // }
//   return TimeInterval(connected.value(), Date());
// }

TimeInterval
BLEDevice::timeIntervalSinceLastPayloadDataUpdate() const
{
  if (!payloadUpdated.has_value()) {
    return TimeInterval::never();
  }
  return TimeInterval(payloadUpdated.value(),Date());
}

// TimeInterval
// BLEDevice::timeIntervalSinceLastWritePayloadSharing() const
// {
//   if (!lastWritePayloadSharingAt.has_value()) {
//     return TimeInterval::never();
//   }
//   return TimeInterval(lastWritePayloadSharingAt.value(),Date());
// }

// TimeInterval
// BLEDevice::timeIntervalSinceLastWritePayload() const
// {
//   if (!lastWritePayloadAt.has_value()) {
//     return TimeInterval::never();
//   }
//   return TimeInterval(lastWritePayloadAt.value(), Date());
// }

// TimeInterval
// BLEDevice::timeIntervalSinceLastWriteRssi() const
// {
//   if (!lastWriteRssiAt.has_value()) {
//     return TimeInterval::never();
//   }
//   return TimeInterval(lastWriteRssiAt.value(),Date());
// }

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
  return flags.state();
}

void
BLEDevice::state(BLEDeviceState newState)
{
  const auto curState = flags.state();
  // Check if failed to connect
  if (BLEDeviceState::disconnected == newState &&
      (BLEDeviceState::disconnected == curState ||
       BLEDeviceState::connecting == curState
      )
  ) {
    ++connectRepeatedFailures;
    if (connectRepeatedFailures >= 5) { // Changed to 5 from 10 for quicker failure in busy areas
      // Ignore for a while (progressive backoff)
      operatingSystem(BLEDeviceOperatingSystem::ignore);
      // Don't backoff again immediately
      connectRepeatedFailures = 0;
    }
  }
  if (BLEDeviceState::connected == newState) {
    flags.hasEverConnected(true);
    connectRepeatedFailures = 0;
  }
  bool changed = curState != newState;
  if (changed) {
    flags.state(newState);
    lastUpdated = Date(); // Constructs Date as now
    delegate->get().device(*this, BLEDeviceAttribute::state);
  }
}

std::optional<BLEDeviceOperatingSystem>
BLEDevice::operatingSystem() const
{
  return flags.operatingSystem();
}

void
BLEDevice::operatingSystem(BLEDeviceOperatingSystem newOS)
{
  lastUpdated = Date(); // Constructs Date as now
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
    lastUpdated = Date(); // Constructs Date as now
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
    lastUpdated = Date(); // Constructs Date as now
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
  // mPayloadCharacteristic.reset();
  // mSignalCharacteristic.reset();
  flags.hasPayloadCharacteristic(false);
  flags.hasLegacyService(false);
  flags.hasSignalCharacteristic(false);
}

void
BLEDevice::registerDiscovery(Date at)
{
  // lastDiscoveredAt.emplace(at);
  lastUpdated = Date(); // TODO verify this shouldn't be 'at' instead (for continuity)
}

// void
// BLEDevice::registerWritePayload(Date at)
// {
//   lastUpdated.emplace(at);
//   lastWritePayloadAt.emplace(at);
// }

// void
// BLEDevice::registerWritePayloadSharing(Date at)
// {
//   lastUpdated.emplace(at);
//   lastWritePayloadSharingAt.emplace(at);
// }

// void
// BLEDevice::registerWriteRssi(Date at)
// {
//   lastUpdated.emplace(at);
//   lastWriteRssiAt.emplace(at);
// }

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

// bool
// BLEDevice::hasServicesSet() const
// {
//   return mServices.has_value();
// }

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