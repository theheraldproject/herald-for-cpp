//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_H
#define HERALD_BLE_H

#include "../datatype/allocatable_array.h"
#include "../datatype/data.h"
#include "ble_mac_address.h"

#include <bitset>
#include <functional>

namespace herald {
namespace ble {

using namespace herald::datatype;

/// MARK: Custom Bluetooth service and characteristic handling definitions

/// \brief Characteristic type individual value constants
/// \since v2.1.0
/// Note: Short value is the internal bitset position (not to be relied upon in application code).
enum class BLECharacteristicTypeValue : short {
  Read = 1,
  WriteWithoutAck = 2,
  WriteWithAck = 3,
  Notify = 4
};

/// \brief Characteristic Type utility class
/// \since v2.1.0
class BLECharacteristicType {
public:
  /// \brief Default constructor. Creates a READ characteristic.
  BLECharacteristicType() noexcept;
  /// \brief Constructor that specified the type to create.
  BLECharacteristicType(const BLECharacteristicTypeValue& value) noexcept;
  /// \brief Default destructor
  ~BLECharacteristicType() noexcept = default;

  /// \brief Merge another characteristic type into this type
  friend BLECharacteristicType& operator|=(BLECharacteristicType& toUpdate, const BLECharacteristicTypeValue& from) noexcept;
  /// \brief Merge two characteristic type values
  friend BLECharacteristicType& operator|=(BLECharacteristicType& toUpdate, const BLECharacteristicType& from) noexcept;

  /// \brief Equality operator for type value
  bool operator==(const BLECharacteristicTypeValue& value) const noexcept;
  /// \brief Inequality operator for type value
  bool operator!=(const BLECharacteristicTypeValue& value) const noexcept;

  /// \brief Equality operator
  bool operator==(const BLECharacteristicType& value) const noexcept;
  /// \brief Inequality operator
  bool operator!=(const BLECharacteristicType& value) const noexcept;
private:
  std::bitset<4> m_type;
};

/// \brief Represents the Bluetooth UUID Size in bits. May be Empty if invalid.
/// \since v2.1.0
enum class BluetoothUUIDSize : short {
  Empty, Short16, Medium32, Long64, Full128
};

/// \brief Represents a Bluetooth Service or Characteristic UUID. May have a size of EMPTY if invalid.
/// \since v2.1.0
class BluetoothUUID {
public:
  /// \brief Default constructor required by array of struct. Creates an EMPTY sized BluetoothUUID with no identity data.
  BluetoothUUID() noexcept;
  /// \brief Constructor that creates a BluetoothUUID from a Data object (which may itself be created from a hex string)
  BluetoothUUID(Data&& from) noexcept;
  /// \brief Conversion constructor from a byte size
  BluetoothUUID(const std::size_t sz) noexcept;
  /// \brief Default destructor
  ~BluetoothUUID() noexcept = default;

  /// \brief Conversion operator to explicit byte size
  explicit operator std::size_t() const noexcept;

  /// \brief The Size of the value. Note if the passed in value is too short, will round the size down. Users MUST checkfor EMPTY size.
  BluetoothUUIDSize size() const noexcept;
  /// \brief Returns the underlying Data object which is guaranteed to be at least (std::size_t)size() bytes long
  const Data& value() const noexcept;

  /// \brief Equality operator
  bool operator==(const BluetoothUUID& other) const noexcept;
  /// \brief Inequality operator
  bool operator!=(const BluetoothUUID& other) const noexcept;
private:
  BluetoothUUIDSize m_size;
  Data m_data;
};

/// \brief Receive a Write request from a remote client for a particular service characteristic. Return true to acknowledge success.
/// \since v2.1.0
using BLEWriteCallback = std::function<bool(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from, const Data& rawWritten, bool expectsAcknowledgement)>;
/// \brief Receive a Notify Subscribe request from a remote client for a particular service characteristic. Return true if subscribed.
/// \since v2.1.0
using BLENotifySubscribeCallback = std::function<bool(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from)>;
/// \brief Receive a Notify UnSubscribe request from a remote client for a particular service characteristic. Return true if unsubscribed.
/// \since v2.1.0
using BLENotifyUnsubscribeCallback = std::function<bool(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from)>;
/// \brief Receive a Read request from a remote client for a particular service characteristic. Return the data to pass back (may be empty).
/// \since v2.1.0
using BLEReadCallback = std::function<Data(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from)>;

const BLEWriteCallback _DummyWriteCallback =
  [](const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from, const Data& rawWritten, bool expectsAcknowledgement) -> bool { return true; };
const BLENotifySubscribeCallback _DummyNotifySubscribeCallback =
  [](const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from) -> bool { return true; };
const BLENotifyUnsubscribeCallback _DummyNotifyUnsubscribeCallback =
  [](const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from) -> bool { return true; };
const BLEReadCallback _DummyReadCallback =
  [](const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLEMacAddress& from) -> Data { return Data(); };

/// \brief A BLE Callbacks struct for use when specifying which callbacks a characteristic supports.
/// \since v2.1.0
/// \note Introduced in v2.1.0 to allow the Notify subscribe functionality of the NordicUARTSensorDelegate implementation service
struct BLECallbacks {
  // TODO consider adding a discovery callback for services we're interested in discovering locally (non-Herald services only)

  /// \brief The callback to call if our advertised characteristic is read by a remote client
  BLEReadCallback readCallback = _DummyReadCallback;
  /// \brief The callback to call if our advertised characteristic is written by a remote client
  BLEWriteCallback writeCallback = _DummyWriteCallback;
  /// \brief The callback to call if our advertised characteristic has been subscribed to by a remote client
  BLENotifySubscribeCallback notifySubscribeCallback = _DummyNotifySubscribeCallback;
  /// \brief The callback to call if our advertised characteristic has been unsubscribed from by a remote client
  BLENotifyUnsubscribeCallback notifyUnsubscribeCallback = _DummyNotifyUnsubscribeCallback;
};

/// \brief Holds a single Characteristic and its callbacks
class BLECharacteristic {
public:
  BLECharacteristic() noexcept;
  BLECharacteristic(BluetoothUUID id, BLECharacteristicType ctype, BLECallbacks cbs) noexcept;
  BLECharacteristic(BLECharacteristic&& other) noexcept;
  BLECharacteristic(const BLECharacteristic& other) noexcept;
  ~BLECharacteristic() noexcept = default;

  BluetoothUUID uuid;
  BLECharacteristicType type;
  BLECallbacks callbacks;

  /// \brief Implicit conversion to BluetoothUUID to enable equality comparison
  operator BluetoothUUID() const noexcept;

  /// \brief Copy assignment operator
  BLECharacteristic& operator=(const BLECharacteristic& from);
  /// \brief Move assignment operator
  BLECharacteristic& operator=(BLECharacteristic&& from);

  /// MARK: Equality and inequality operators
  bool operator==(const BLECharacteristic& other) const noexcept;
  bool operator!=(const BLECharacteristic& other) const noexcept;
  bool operator==(const BluetoothUUID& id) const noexcept;
  bool operator!=(const BluetoothUUID& id) const noexcept;
  bool operator==(const BLECharacteristicType& characteristicType) const noexcept;
  bool operator!=(const BLECharacteristicType& characteristicType) const noexcept;

  /// MARK: Assignment operators that merge in characteristics
  friend BLECharacteristic& operator|=(BLECharacteristic& toUpdate, const BLECharacteristic& toMerge) noexcept;
  friend BLECharacteristic& operator|=(BLECharacteristic& toUpdate, const BLECharacteristicType& toMerge) noexcept;
};

/// \brief Holds a list of Bluetooth Characteristics
#ifndef HERALD_BLECHARACTERISTICLIST_MAX
using BLECharacteristicList = AllocatableArray<BLECharacteristic, 8, true>;
#else
using BLECharacteristicList = AllocatableArray<BLECharacteristic, HERALD_BLECHARACTERISTICLIST_MAX, true>;
#endif

/// \brief Holds a single Service and its Characteristics
class BLEService {
public:
  BLEService() noexcept;
  BLEService(BluetoothUUID id,BLECharacteristicList cl) noexcept;
  BLEService(BLEService&& other) noexcept;
  BLEService(const BLEService& other) noexcept;
  ~BLEService() noexcept = default;
  
  BluetoothUUID uuid;
  BLECharacteristicList characteristics;

  /// \brief Implicit conversion to BluetoothUUID to enable equality comparison
  operator BluetoothUUID() const noexcept;

  /// \brief Copy assignment operator
  BLEService& operator=(const BLEService& from) noexcept;
  /// \brief Move assignment operator
  BLEService& operator=(BLEService&& from) noexcept;

  /// MARK: Equality and inequality operators
  bool operator==(const BluetoothUUID& id) const noexcept;
  bool operator!=(const BluetoothUUID& id) const noexcept;
  bool operator==(const BLEService& other) const noexcept;
  bool operator!=(const BLEService& other) const noexcept;

  /// MARK: Assignment operators that append characteristic(s)
  friend BLEService& operator|=(BLEService& toUpdate, BLEService& toMerge) noexcept;
  friend BLEService& operator|=(BLEService& toUpdate, const BLECharacteristic& toMerge) noexcept;
};

/// \brief Holds a list of Bluetooth Services, including their Characteristics
#ifndef HERALD_BLESERVICELIST_MAX
using BLEServiceList = AllocatableArray<BLEService, 8, true>;
#else
using BLEServiceList = AllocatableArray<BLEService, HERALD_BLESERVICELIST_MAX, true>;
#endif

}
}

#endif