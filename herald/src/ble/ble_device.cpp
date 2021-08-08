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

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;

BLEDeviceFlags::BLEDeviceFlags() : bitFields(0) /* empty */
{
  reset();
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

  bitFields.set(9);
  bitFields.set(10);
  bitFields.set(11);
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
  return BLEInternalState::discovered;
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

BLEDeviceState
BLEDeviceFlags::state() const
{
  if (bitFields.test(3)) {
    if (bitFields.test(4)) {
      return BLEDeviceState::uninitialised;
    } else {
      return BLEDeviceState::connecting;
    }
  } else {
    if (bitFields.test(4)) {
      return BLEDeviceState::connected;
    } else {
      return BLEDeviceState::disconnected;
    }
  }
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


BLEDeviceOperatingSystem
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
  return BLEDeviceOperatingSystem::unknown;
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
  return bitFields.test(9) || bitFields.test(10) || bitFields.test(11);

}

BLELegacyService
BLEDeviceFlags::legacyService() const
{
  if (hasHeraldService()) {
    return BLELegacyService::NotApplicable;
  }
  if (bitFields.test(9)) {
    if (bitFields.test(10)) {
      if (bitFields.test(11)) {
        return BLELegacyService::Unknown;
      } else {
        return BLELegacyService::OpenTrace;
      }
    } else {
      if (bitFields.test(11)) {
        return BLELegacyService::AustraliaCovidSafe;
      }
    }
  }
  return BLELegacyService::Unknown;
}

void
BLEDeviceFlags::legacyService(BLELegacyService newValue)
{
  switch (newValue)
  {
  case BLELegacyService::NotApplicable:
    bitFields.set(9,true);
    bitFields.set(10,true);
    bitFields.set(11,true);
    break;
  case BLELegacyService::Unknown:
    bitFields.set(9,true);
    bitFields.set(10,true);
    bitFields.set(11,true);
    break;
  case BLELegacyService::OpenTrace:
    bitFields.set(9,true);
    bitFields.set(10,true);
    bitFields.set(11,false);
    break;
  case BLELegacyService::AustraliaCovidSafe:
    bitFields.set(9,true);
    bitFields.set(10,false);
    bitFields.set(11,true);
    break;
  }
}

bool
BLEDeviceFlags::hasPayloadCharacteristic() const
{
  return bitFields.test(12);
}

void
BLEDeviceFlags::hasPayloadCharacteristic(bool newValue)
{
  bitFields.set(12,newValue);
}

bool
BLEDeviceFlags::signalCharacteristic() const
{
  return bitFields.test(13);
}

void
BLEDeviceFlags::signalCharacteristic(bool newValue)
{
  bitFields.set(13,newValue);
}

bool
BLEDeviceFlags::hasSecureCharacteristic() const
{
  return bitFields.test(14);
}

void
BLEDeviceFlags::hasSecureCharacteristic(bool newValue)
{
  bitFields.set(14,newValue);
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
    conf(staticConfig),
    delegate(std::nullopt),
    id(),
    flags(),
    lastUpdated(Date(0)),
    stateData(std::monostate()),
    payload(),
    mRssi(0)
{
  ;
}



BLEDevice::BLEDevice(BLESensorConfiguration& config)
  : Device(),
    conf(config),
    delegate(std::nullopt),
    id(),
    flags(),
    lastUpdated(Date(0)),
    stateData(std::monostate()),
    payload(),
    mRssi(0)
{
  ;
}

BLEDevice::BLEDevice(BLESensorConfiguration& config, TargetIdentifier identifier, BLEDeviceDelegate& del,
  const Date& createdAt)
  : Device(),
    conf(config),
    delegate(std::make_optional(std::reference_wrapper<BLEDeviceDelegate>(del))),
    id(identifier),
    flags(),
    lastUpdated(createdAt),
    stateData(DiscoveredState()),
    payload(),
    mRssi(0)
{
  ;
}

BLEDevice::BLEDevice(const BLEDevice& other)
  : Device(),
    conf(other.conf),
    delegate(other.delegate),
    id(other.id),
    flags(other.flags),
    lastUpdated(other.lastUpdated),
    stateData(other.stateData),
    payload(other.payload),
    mRssi(other.mRssi)
{
  ;
}

BLEDevice::~BLEDevice() = default;

void
BLEDevice::reset(const TargetIdentifier& newID, BLEDeviceDelegate& newDelegate)
{
  delegate.emplace(std::reference_wrapper<BLEDeviceDelegate>(newDelegate));
  id = newID;
  stateData = DiscoveredState();
  flags.internalState(BLEInternalState::discovered);
  flags.state(BLEDeviceState::disconnected); // allows action from protocol providers (i.e. no longer uninitialised)
  payload.clear();
  mRssi = 0;
}

BLEDevice&
BLEDevice::operator=(const BLEDevice& other)
{
  conf = other.conf;
  delegate = other.delegate;
  id = other.id;
  flags = other.flags;
  lastUpdated = other.lastUpdated;
  stateData = other.stateData;
  payload = other.payload;
  mRssi = other.mRssi;
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
  // if (std::monostate == stateData) {
  //   return TimeInterval::zero();
  // }
  return TimeInterval(lastUpdated, Date());;
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
  if (BLEInternalState::identified != flags.internalState()) {
    return TimeInterval::never();
  }
  return TimeInterval(std::get<RelevantState>(stateData).payloadUpdated,Date());
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
  const auto is = flags.internalState();
  if (is != BLEInternalState::relevant &&
      is != BLEInternalState::identified) {
    return TimeInterval::zero();
  }
  const auto ignoreUntil = std::get<RelevantState>(stateData).ignoreUntil;
  if (ignoreUntil == TimeInterval::never()) {
    return TimeInterval::never();
  }
  return TimeInterval(Date(),ignoreUntil);
}

// property getters and setters
std::optional<BLEMacAddress>
BLEDevice::pseudoDeviceAddress() const
{
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    return {};
  }
  return std::make_optional(std::get<RelevantState>(stateData).pseudoAddress);
}

void
BLEDevice::pseudoDeviceAddress(BLEMacAddress newAddress)
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    flags.internalState(BLEInternalState::relevant);
    stateData = RelevantState{};
  }
  const auto pa = pseudoDeviceAddress();
  if (!pa.has_value() || pa.value() != newAddress) {
    std::get<RelevantState>(stateData).pseudoAddress = newAddress;
    lastUpdated.setToNow(); // Constructs Date as now
  }
}

BLEDeviceState
BLEDevice::state() const
{
  return flags.state();
}

void
BLEDevice::state(BLEDeviceState newState)
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    flags.internalState(BLEInternalState::relevant);
    stateData = RelevantState{};
  }
  auto& rs = std::get<RelevantState>(stateData);

  const auto curState = flags.state();
  // Check if failed to connect
  if (BLEDeviceState::disconnected == newState &&
      (BLEDeviceState::disconnected == curState ||
       BLEDeviceState::connecting == curState
      )
  ) {
    ++rs.connectRepeatedFailures;
    if (rs.connectRepeatedFailures >= 5) { // Changed to 5 from 10 for quicker failure in busy areas
      // Ignore for a while (progressive backoff)
      operatingSystem(BLEDeviceOperatingSystem::ignore);
      // Don't backoff again immediately
      rs.connectRepeatedFailures = 0;
    }
  }
  if (BLEDeviceState::connected == newState) {
    flags.hasEverConnected(true);
    rs.connectRepeatedFailures = 0;
  }
  bool changed = curState != newState;
  if (changed) {
    flags.state(newState);
    lastUpdated = Date(); // Constructs Date as now
    if (delegate.has_value()) {
      delegate->get().device(*this, BLEDeviceAttribute::state);
    }
  }
}

BLEDeviceOperatingSystem
BLEDevice::operatingSystem() const
{
  return flags.operatingSystem();
}

void
BLEDevice::operatingSystem(BLEDeviceOperatingSystem newOS)
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    flags.internalState(BLEInternalState::relevant);
    stateData = RelevantState{};
  }
  auto& rs = std::get<RelevantState>(stateData);

  lastUpdated.setToNow();
  const auto os = operatingSystem();
  if (os != BLEDeviceOperatingSystem::unknown && os == BLEDeviceOperatingSystem::ignore) {
    if (TimeInterval::zero() == rs.ignoreForDuration) {
      rs.ignoreForDuration = TimeInterval::minutes(1);
    } else if (rs.ignoreForDuration < TimeInterval::minutes(3)) {
      // progressive backoff for unknown device
      rs.ignoreForDuration *= 1.2;
      if (rs.ignoreForDuration > TimeInterval::minutes(7)) {
        // just ignore as the mac will have rotated (7:43 will occur half way through 15 mins intervals)
        // As the total BLE DB timeout is ~ 25 minutes, this will save significant connection attempt cycles
        // mIgnore = true;
        // Change to ignored (aka filtered) state
        flags.internalState(BLEInternalState::filtered);
      }
    }
    rs.ignoreUntil = lastUpdated + rs.ignoreForDuration;
  } else {
    rs.ignoreUntil = Date(0); // ensure we've already passed the time
  }
  if (os == BLEDeviceOperatingSystem::ios || os == BLEDeviceOperatingSystem::android) {
    rs.ignoreForDuration = TimeInterval::zero();
  }
  bool changed = (BLEDeviceOperatingSystem::unknown != os) || os != newOS;
  if (changed) {
    flags.operatingSystem(newOS);
    delegate->get().device(*this, BLEDeviceAttribute::operatingSystem);
  }
}

PayloadData
BLEDevice::payloadData() const
{
  return payload;
}

void
BLEDevice::payloadData(PayloadData newPayloadData)
{
  if (flags.internalState() != BLEInternalState::identified) {
    flags.internalState(BLEInternalState::identified);
    stateData = RelevantState{};
  }
  bool changed = payload.size() == 0 || payload != newPayloadData;
  if (changed) {
    payload = newPayloadData;
    lastUpdated.setToNow(); // Constructs Date as now
    std::get<RelevantState>(stateData).payloadUpdated.setToNow();
    // payloadUpdated.emplace();
    if (delegate.has_value()) {
      delegate.value().get().device(*this, BLEDeviceAttribute::payloadData);
    }
  }
}

// std::optional<ImmediateSendData>
// BLEDevice::immediateSendData() const
// {
//   return mImmediateSendData;
// }

// void
// BLEDevice::immediateSendData(ImmediateSendData toSend)
// {
//   bool changed = !mImmediateSendData.has_value() || mImmediateSendData.value() != toSend;
//   if (changed) {
//     mImmediateSendData.emplace(toSend);
//     lastUpdated = Date(); // Constructs Date as now
//     delegate->get().device(*this, BLEDeviceAttribute::immediateSendData);
//   }
// }

// void
// BLEDevice::clearImmediateSendData()
// {
//   mImmediateSendData.reset();
// }

RSSI
BLEDevice::rssi() const
{
  return mRssi;
}

void
BLEDevice::rssi(RSSI newRSSI)
{
  bool changed = (0 == mRssi.intValue()) || (mRssi != newRSSI);
  if (changed) {
    mRssi = newRSSI;
    // lastUpdated.emplace(); // Constructs Date as now
    if (delegate.has_value()) {
      delegate.value().get().device(*this, BLEDeviceAttribute::rssi);
    }
  }
}

std::optional<BLETxPower>
BLEDevice::txPower() const
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    return {};
  }
  auto& rs = std::get<RelevantState>(stateData);
  return std::make_optional(rs.txPower);
}

void
BLEDevice::txPower(BLETxPower newPower)
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    // return; // TODO convert type to Relevant instead?
    flags.internalState(BLEInternalState::relevant);
    stateData = RelevantState{};
  }
  auto& rs = std::get<RelevantState>(stateData);
  bool changed = rs.txPower != newPower;
  if (changed) {
    rs.txPower = newPower;
    lastUpdated.setToNow();
    delegate->get().device(*this, BLEDeviceAttribute::txPower);
  }
}

// bool
// BLEDevice::receiveOnly() const
// {
//   return mReceiveOnly;
// }

// void
// BLEDevice::receiveOnly(bool newReceiveOnly)
// {
//   mReceiveOnly = newReceiveOnly;
// }

std::optional<UUID>
BLEDevice::signalCharacteristic() const
{
  // Safety check
  const auto is = flags.internalState();
  if (is == BLEInternalState::discovered ||
      is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    // Apple devices are not spec compliant for Bluetooth, which is what this value really means
    if (SignalCharacteristicType::SpecCompliant == flags.signalCharacteristic()) {
      return std::make_optional(conf.androidSignalCharacteristicUUID);
    }
    return std::make_optional(conf.iosSignalCharacteristicUUID);
  }
  return {};
}

void
BLEDevice::signalCharacteristic(UUID newChar)
{
  if (conf.androidSignalCharacteristicUUID == newChar) {
    flags.signalCharacteristic(true);
  } else {
    flags.signalCharacteristic(false);
  }
  lastUpdated.setToNow();
}

std::optional<UUID>
BLEDevice::payloadCharacteristic() const
{
  if (!flags.hasPayloadCharacteristic()) {
    return {};
  }
  return std::make_optional(conf.payloadCharacteristicUUID);
}

void
BLEDevice::payloadCharacteristic(UUID newChar)
{
  if (conf.payloadCharacteristicUUID == newChar) {
    flags.hasPayloadCharacteristic(true);
  }
  lastUpdated.setToNow();
}

// State engine methods
bool
BLEDevice::ignore() const
{
  const auto is = flags.internalState();
  if (is == BLEInternalState::filtered ||
      is == BLEInternalState::timed_out) {
    return true;
  }
  if (is == BLEInternalState::discovered) {
    return false;
  }
  auto& rs = std::get<RelevantState>(stateData);

  // Check for timed ignore
  if (0 == rs.ignoreUntil) {
    return false;
  }
  if (Date() < rs.ignoreUntil) {
    return true;
  }
  return false;
}

void
BLEDevice::ignore(bool newIgnore) // set permanent ignore flag
{
  // change state to ignored permanently
  if (newIgnore) {
    stateData = FilteredState(); // TODO determine if this breaks end of life detection for android devices with pseudo device address
    flags.internalState(BLEInternalState::filtered);
  }
  lastUpdated.setToNow();
}

void
BLEDevice::invalidateCharacteristics()
{
  // mPayloadCharacteristic.reset();
  // mSignalCharacteristic.reset();
  flags.hasPayloadCharacteristic(false);
  flags.legacyService(BLELegacyService::Unknown);
  flags.signalCharacteristic(SignalCharacteristicType::SpecCompliant); // Assume spec compliant
  lastUpdated.setToNow();
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

// bool
// BLEDevice::hasAdvertData() const
// {
//   const auto is = flags.internalState();
//   if (is == BLEInternalState::discovered) {
//     return true;
//   }
// }

void
BLEDevice::advertData(std::vector<BLEAdvertSegment> newSegments)
{
  stateData = DiscoveredState{newSegments};
  flags.internalState(BLEInternalState::discovered);
}

// const std::vector<BLEAdvertSegment>&
// BLEDevice::advertData() const
// {
//   const auto is = flags.internalState();
//   if (is != BLEInternalState::discovered) {
//     return {};
//   }
//   auto& rs = std::get<DiscoveredState>(stateData);
//   return rs.segments;
// }

// bool
// BLEDevice::hasServicesSet() const
// {
//   return mServices.has_value();
// }

void
BLEDevice::services(std::vector<UUID> services)
{
  lastUpdated.setToNow();
  for (auto& svc : services) {
    if (svc == conf.serviceUUID) {
      flags.hasHeraldService(true);
      return;
    }
    if (svc == herald::ble::legacyOpenTraceUUID) {
      flags.legacyService(BLELegacyService::OpenTrace);
      return;
    }
    if (svc == herald::ble::legacyAustraliaServiceUUID) {
      flags.legacyService(BLELegacyService::AustraliaCovidSafe);
      return;
    }
  }
  flags.hasHeraldService(false);
}

bool
BLEDevice::hasService(const UUID& serviceUUID) const
{
  if (flags.hasHeraldService()) {
    if (conf.serviceUUID == serviceUUID) {
      return true;
    }
  }
  return false;
}

const BLESensorConfiguration&
BLEDevice::configuration() const noexcept
{
  return conf;
}


BLESensorConfiguration
BLEDevice::staticConfig = BLESensorConfiguration();

}
}