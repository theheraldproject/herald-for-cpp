//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_DEVICE_H
#define HERALD_BLE_DEVICE_H

#include "ble_tx_power.h"
#include "ble_mac_address.h"
#include "ble_sensor_configuration.h"

#include "../device.h"

#include "../datatype/allocatable_array.h"
#include "../datatype/payload_data.h"
#include "../datatype/payload_sharing_data.h"
#include "../datatype/immediate_send_data.h"
#include "../datatype/target_identifier.h"
#include "../datatype/time_interval.h"
#include "../datatype/date.h"
#include "../datatype/uuid.h"
#include "filter/ble_advert_parser.h"

#include <variant>
#include <vector>
#include <optional>
#include <bitset>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;

class BLEDeviceDelegate; // fwd decl

enum class BLEDeviceAttribute : int {
  peripheral, state, operatingSystem, payloadData, rssi, txPower, immediateSendData
};

enum class BLEDeviceOperatingSystem : int {
  android_tbc, android, ios_tbc, ios, ignore, shared, unknown
};

/// \brief Low-level Bluetooth status
enum class BLEDeviceState : int {
  uninitialised, /* Uninitialised state only seen within Herald, not seen by those listing devices in a DB */
  connecting, connected, disconnected
};

/// \brief Internal discovery BLE state to aid efficient memory use in BLEDevice.
enum class BLEInternalState : short {
  /// \brief Discovered via Advert, but advert not assessed yet
  discovered,
  /// \brief Advert assessed, but device filtered as irrelevant
  filtered,
  /// \brief Advert assessed, and device requires further inspection (via connect and list services/characteristics)
  has_potential,
  /// \brief Determined to be a relevant device (Herald, or other relevant protocol)
  relevant,
  /// \brief Payload has been retrieved
  identified,
  /// \brief Not seen in a while, memory slot eligible for reallocation
  timed_out
};

enum class BLELegacyService : short {
  NotApplicable, // i.e. Herald
  Unknown, // i.e. other
  OpenTrace,
  AustraliaCovidSafe
  // TODO others later (up to 7 values plus Unknown and NotApplicable)
};

namespace SignalCharacteristicType {
  constexpr bool SpecCompliant = true;
  constexpr bool NotSpecCompliant = false;
}

/// \brief INTERNAL Herald class used to minimise the memory footprint with hundreds of devices nearby
class BLEDeviceFlags {
public:
  BLEDeviceFlags();
  ~BLEDeviceFlags() = default;

  void reset();

  BLEInternalState internalState() const;
  std::string internalStateDescription() const;
  void internalState(BLEInternalState newInternalState);

  BLEDeviceState state() const;
  void state(BLEDeviceState newState);

  BLEDeviceOperatingSystem operatingSystem() const;
  void operatingSystem(BLEDeviceOperatingSystem newOS);

  bool hasHeraldService() const;
  void hasHeraldService(bool newValue);
  bool hasLegacyService() const;
  BLELegacyService legacyService() const;
  void legacyService(BLELegacyService newValue);
  bool hasPayloadCharacteristic() const;
  void hasPayloadCharacteristic(bool newValue);
  bool signalCharacteristic() const;
  void signalCharacteristic(bool newValue);
  bool hasSecureCharacteristic() const;
  void hasSecureCharacteristic(bool newValue);
  bool hasEverConnected() const;
  void hasEverConnected(bool newValue);

private:
  // Note: Bit fields merged into a single class
  //unsigned short int bitfields; // at least 16 bits (usually always 16 bits)
  std::bitset<16> bitFields; // will always consume sets of 8 bits, so round up
  // 0 = BLEInternalState field 1
  // 1 = BLEInternalState field 2
  // 2 = BLEInternalState field 3
  // 3 = BLEDeviceState field 1
  // 4 = BLEDeviceState field 2
  // 5 = BLEDeviceOperatingSystem field 1
  // 6 = BLEDeviceOperatingSystem field 2
  // 7 = BLEDeviceOperatingSystem field 3
  // 8 = hasHeraldService
  // 9 = hasLegacyService field 1
  // 10 = hasLegacyService field 2
  // 11 = hasLegacyService field 3
  // 12 = hasPayloadChar
  // 13 = hasSignalChar
  // 14 = hasSecureChar
  // 15 = hasEverConnected
};

struct DiscoveredState {
  std::vector<BLEAdvertSegment> segments; // TODO change to fixed size containers
};

using FilteredState = std::monostate;

// struct HasPotentialState {
//   BLEDeviceState state;
//   BLEDeviceOperatingSystem os;
//   BLETxPower txPower;

//   TimeInterval ignoreForDuration; // QN isn't this used in fetch payload stage?
//   Date ignoreUntil; // QN isn't this used in fetch payload stage?

//   std::vector<UUID> services; // TODO can we not just make this ephemeral?
//   bool hasEverConnected;
//   int connectRepeatedFailures;
// };

struct RelevantState {
  RelevantState()
   : txPower(0), 
     ignoreForDuration(0),
     ignoreUntil(0),
     pseudoAddress(),
     connectRepeatedFailures(0),
     payloadUpdated(0)
  {}
  
  BLETxPower txPower;
  
  TimeInterval ignoreForDuration; // Isn't this TIME^connectRepeatedFailures?
  Date ignoreUntil;

  BLEMacAddress pseudoAddress; // all zeros if unset (also OS is not Android)

  unsigned short int connectRepeatedFailures;

  Date payloadUpdated; // Note only use if state is Identified, not just Relevant
};

// struct IdentifiedState {
//   BLETxPower txPower;
//   BLEMacAddress pseudoAddress; // QN Isn't this available from advert stage?

//   unsigned short  connectRepeatedFailures;
// };


class BLEDevice : public Device {
public:
  BLEDevice(); // Used for array initialisation, uses dummy static config ONLY
  BLEDevice(BLESensorConfiguration& config);
  BLEDevice(BLESensorConfiguration& config,BLEDeviceDelegate& delegate, const Date& created = Date()); // default (uninitialised state) constructor
  BLEDevice(BLESensorConfiguration& config,TargetIdentifier identifier, BLEDeviceDelegate& delegate, const Date& created = Date());
  BLEDevice(const BLEDevice& other); // copy ctor
  BLEDevice(BLEDevice&& other) = delete; // remove move constructor
  ~BLEDevice();

  /// \brief Resets the device to what it's state would be if just constructed. Allows re-use in fixed size containers
  void reset(const TargetIdentifier& newID, BLEDeviceDelegate& newDelegate);

  BLEDevice& operator=(const BLEDevice& other); // copy assign
  BLEDevice operator=(BLEDevice && other) = delete; // remove mode assign
  
  bool operator==(const BLEDevice& other) const noexcept;
  bool operator!=(const BLEDevice& other) const noexcept;

  /// \brief Returns the BLESensorConfiguration reference relating to this instance
  const BLESensorConfiguration& configuration() const noexcept;

  const TargetIdentifier& identifier() const override; // MAC ADDRESS OR PSEUDO DEVICE ADDRESS
  void identifier(const TargetIdentifier&) override; // MAC ADDRESS OR PSEUDO DEVICE ADDRESS
  // Date created() const override; // TODO unused, consider removing

  // basic descriptors
  std::string description() const;
  operator std::string() const;

  // GENERAL BLUETOOTH STATE
  TimeInterval timeIntervalSinceLastUpdate() const override;
  // TimeInterval timeIntervalSinceConnected() const; // TODO unused, consider removing

  // TODO add in generic Advert and GATT handle number information caching here

  // bool hasAdvertData() const; // TODO unused, consider removing
  void advertData(std::vector<BLEAdvertSegment> segments); // TODO getter unused, so consider removing
  // const std::vector<BLEAdvertSegment>& advertData() const; // TODO unused, consider removing

  /** Have we set the service list for this device yet? (i.e. done GATT service discover) **/
  // bool hasServicesSet() const; // TODO unused, consider removing
  /** Set services found on this device (set, not append) **/
  void services(std::vector<UUID> services);
  /** Does the service list contain a service UUID? **/
  bool hasService(const UUID& serviceUUID) const;

  BLEDeviceState state() const;
  void state(BLEDeviceState newState);

  std::string internalStateDescription() const;

  // TODO decide if operatingSystem is relevant anymore??? - change it to BluetoothComplianceFlag?
  BLEDeviceOperatingSystem operatingSystem() const;
  void operatingSystem(BLEDeviceOperatingSystem newOS);

  RSSI rssi() const;
  void rssi(RSSI newRSSI);

  std::optional<BLETxPower> txPower() const;
  void txPower(BLETxPower newPower);

  // NOTE May need a herald specific version of this, as we may wish a herald device to only receive or transmit
  // bool receiveOnly() const;
  // void receiveOnly(bool newReceiveOnly);




  // HERALD PROTOCOL SPECIFIC STATE - TODO HIDE THESE FROM SENSOR/EXTERNAL CALLS

  // timing related getters
  TimeInterval timeIntervalSinceLastPayloadDataUpdate() const; // TODO unused, consider removing
  TimeInterval timeIntervalSinceLastWritePayloadSharing() const; // TODO unused, consider removing
  TimeInterval timeIntervalSinceLastWritePayload() const; // TODO unused, consider removing
  TimeInterval timeIntervalSinceLastWriteRssi() const; // TODO unused, consider removing
  TimeInterval timeIntervalUntilIgnoreExpired() const; // TODO unused, consider removing

  std::optional<UUID> signalCharacteristic() const;
  void signalCharacteristic(UUID newChar);

  std::optional<UUID> payloadCharacteristic() const; // TODO unused, consider removing (Needed for interop?)
  void payloadCharacteristic(UUID newChar); // TODO unused, consider removing (Needed for interop?)

  std::optional<BLEMacAddress> pseudoDeviceAddress() const;
  void pseudoDeviceAddress(BLEMacAddress newAddress);

  PayloadData payloadData() const;
  void payloadData(PayloadData newPayloadData);

  // std::optional<ImmediateSendData> immediateSendData() const;
  // void immediateSendData(ImmediateSendData toSend);
  // void clearImmediateSendData();

  // State engine methods - Herald specific
  bool ignore() const;
  void ignore(bool newIgnore);
  void invalidateCharacteristics();
  void registerDiscovery(Date at); // ALWAYS externalise time (now())
  // void registerWritePayload(Date at); // ALWAYS externalise time (now())
  // void registerWritePayloadSharing(Date at); // ALWAYS externalise time (now())
  // void registerWriteRssi(Date at); // ALWAYS externalise time (now())
  
private:
  static BLESensorConfiguration staticConfig; // Used by empty constructor for array construction ONLY
  BLESensorConfiguration& conf; // allows this class to minimise its memory storage space

  std::optional<std::reference_wrapper<BLEDeviceDelegate>> delegate; // Optional to avoid catch-22
  TargetIdentifier id; // Mac address // TODO replace this here with a hash

  BLEDeviceFlags flags; // merging of several enums and fields to save memory
  // BLEInternalState internalState; // used for selection of union below
  // BLEDeviceOperatingSystem os;
  // BLEDeviceState state;

  // Data holders
  // Date mCreated;
  // std::optional<Date> lastUpdated;
  Date lastUpdated; // Merges mCreated and lastUpdated


  // \brief RelevantState is used for hasPotential, relevant and identified
  std::variant<DiscoveredState, FilteredState, RelevantState> stateData;

  // std::optional<BLEDeviceState> mState; // hasPotential, relevant, identified
  // std::optional<BLEDeviceOperatingSystem> os; // hasPotential, relevant, identified
  PayloadData payload; // TODO make ephemeral (other than ID portion) - identified
  // std::optional<ImmediateSendData> mImmediateSendData; // TODO make ephemeral - identified
  RSSI mRssi; // TODO make ephemeral - all
  // std::optional<BLETxPower> mTxPower; // hasPotential, relevant, identified
  // bool mReceiveOnly; // TODO make convenience method based on other settings

  // // TODO simplify these three into their own state option (union)
  // bool mIgnore; // filtered
  // std::optional<TimeInterval> ignoreForDuration; // hasPotential
  // std::optional<Date> ignoreUntil; // hasPotential

  // // TODO key these to an external limited list of characteristics
  // std::optional<UUID> mPayloadCharacteristic; // relevant
  // std::optional<UUID> mSignalCharacteristic; // relevant
  // std::optional<BLEMacAddress> pseudoAddress; // relevant, identified

  // // Remove these as we don't support writing from C++ (there's no need - read works solidly)
  // std::optional<Date> lastWriteRssiAt;
  // std::optional<Date> lastWritePayloadAt;
  // std::optional<Date> lastWritePayloadSharingAt;

  // // std::optional<Date> lastDiscoveredAt; // TODO remove - This is always the same as the last RSSI (i.e. lastUpdated) date
  // std::optional<Date> connected; // merge with hasEverConnected
  // std::optional<Date> payloadUpdated;

  // // TODO make this ephemeral
  // std::optional<std::vector<BLEAdvertSegment>> segments; // discovered
  // // TODO limit this to a key to the detected service ID (Herald or other. Prefer Herald)
  // std::optional<std::vector<UUID>> mServices; // hasPotential, relevant

  // bool hasEverConnected; // hasPotential, relevant
  // int connectRepeatedFailures; // hasPotential, relevant
};

using BLEDeviceList = ReferenceArray<BLEDevice, 150, true>;

} // end namespace
} // end namespace

namespace std {
// Equality comparison operators for std::optional<BLEDevice>

/// \brief Equality operator overload for BLEDevice in std::reference_wrapper
bool operator==(const std::reference_wrapper<herald::ble::BLEDevice>& lhs, const std::reference_wrapper<herald::ble::BLEDevice>& rhs) noexcept;

/// \brief Inequality operator overload for BLEDevice in std::reference_wrapper
bool operator!=(const std::reference_wrapper<herald::ble::BLEDevice>& lhs, const std::reference_wrapper<herald::ble::BLEDevice>& rhs) noexcept;

}

#endif