//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLUETOOTH_STATE_MANAGER_H
#define HERALD_BLUETOOTH_STATE_MANAGER_H

#include "../datatype/allocatable_array.h"
#include "../datatype/bluetooth_state.h"
#include "../datatype/data.h"
#include "../datatype/proximity.h"
#include "ble_mac_address.h"
#include "bluetooth_state_manager_delegate.h"

#include <bitset>
#include <functional>

namespace herald {
namespace ble {

using namespace herald::datatype;

/// MARK: Custom Bluetooth service and characteristic handling definitions

/// \brief Characteristic type individual value constants
/// \since v2.1.0
/// Note: Short value is the internal bitset position (not to be relied upon in application code).
enum class CharacteristicTypeValue : short {
  READ = 1,
  WRITE_WITHOUT_ACK = 2,
  WRITE_WITH_ACK = 3,
  NOTIFY = 4
};

/// \brief Characteristic Type utility class
/// \since v2.1.0
class CharacteristicType {
public:
  CharacteristicType();
  CharacteristicType(const CharacteristicTypeValue& value);
  ~CharacteristicType() = default;

  CharacteristicType& operator=(const CharacteristicTypeValue& from);

  bool operator==(const CharacteristicTypeValue& value) const;
  bool operator!=(const CharacteristicTypeValue& value) const;
private:
  std::bitset<4> m_type;
};

/// \brief Represents the Bluetooth UUID Size in bits. May be Empty if invalid.
/// \since v2.1.0
enum class BluetoothUUIDSize : short {
  EMPTY, SHORT_16, MEDIUM_32, LONG_64, FULL_128

};

/// \brief Represents a Bluetooth Service or Characteristic UUID. May have a size of EMPTY if invalid.
/// \since v2.1.0
class BluetoothUUID {
public:
  /// \brief Constructor that creates a BluetoothUUID from a Data object (which may itself be created from a hex string)
  BluetoothUUID(Data&& from);
  /// \brief Conversion constructor from a byte size
  BluetoothUUID(const std::size_t sz);
  /// \brief Default destructor
  ~BluetoothUUID() = default;

  /// \brief Conversion operator to explicit byte size
  explicit operator std::size_t() const;

  /// \brief The Size of the value. Note if the passed in value is too short, will round the size down. Users MUST checkfor EMPTY size.
  BluetoothUUIDSize size() const;
  /// \brief Returns the underlying Data object which is guaranteed to be at least (std::size_t)size() bytes long
  const Data& value() const;
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
struct BLECallbacks {
  BLEReadCallback readCallback = _DummyReadCallback;
  BLEWriteCallback writeCallback = _DummyWriteCallback;
  BLENotifySubscribeCallback notifySubscribeCallback = _DummyNotifySubscribeCallback;
  BLENotifyUnsubscribeCallback notifyUnsubscribeCallback = _DummyNotifyUnsubscribeCallback;
};

/// \brief Holds a single Characteristic and its callbacks
struct BLECharacteristic {
  CharacteristicType type;
  BluetoothUUID uuid;
  BLECallbacks callbacks;

  // TODO equality and inequality operators
};

/// \brief Holds a list of Bluetooth Characteristics
using BLECharacteristicList = AllocatableArray<BLECharacteristic, 8, true>;

/// \brief Holds a single Service and its Characteristics
struct BLEService {
  BluetoothUUID uuid;
  BLECharacteristicList characteristics;

  // TODO equality and inequality operators
  // TODO assignment operator that appends characteristic(s) if the uuids of the services match
};

/// \brief Holds a list of Bluetooth Services, including their Characteristics
using BLEServiceList = AllocatableArray<BLEService, 8, true>;

// /// \brief Holds a fixed (max) size list of characteristics and their callbacks
// /// \since v2.1.0
// template <std::size_t Size>
// class BLECharacteristicList {
// public:
//   static constexpr std::size_t MaxSize = Size;
//   BLECharacteristicList() : m_chars() {};
//   ~BLECharacteristicList() = default;

//   bool addCharacteristic(BLECharacteristic&& toAdd) {

//   }

//   void removeCharacteristic(const BluetoothUUID& toRemove) {

//   }

// private:
//   AllocatableArray<BLECharacteristic, MaxSize> m_chars;
// };

// /// \brief Holds a fixed (max) size list of services, their characteristics, and those characteristics' callbacks
// /// \since v2.1.0
// template <std::size_t Size>
// class BLEServiceList {
// public:
//   static constexpr std::size_t MaxSize = Size;

// private:
//   AllocatableArray<BLEService, MaxSize> m_services;
// };




/// MARK: Platform independent Bluetooth state handling

/// \brief Tagging interface for managing low level Bluetooth state, but independent of platform specific callbacks/classes (for portability)
class BluetoothStateManager {
public:
  /// \brief Default constructor
  BluetoothStateManager() = default;
  /// \brief Default destructor
  virtual ~BluetoothStateManager() = default;



  /// MARK: Bluetooth device state delegate methods

  /// \brief Adds a delegate that is called when this Bluetooth device's state changes
  virtual void add(BluetoothStateManagerDelegate& delegate) = 0;

  /// \brief Retrieves the current state of this Bluetooth device
  virtual BluetoothState state() = 0;



  /// MARK: Custom (external to Herald) platform independent service definition and interactions

  /// \brief Request to add a custom service advertisement. Returns false if it fails (E.g. exceeded advertisement data size). Assumes a low-level OS handler is managing the characteristic callbacks.
  /// \since v2.1.0
  /// \note MUST guarantee that the same service is not advertised multiple times. Does not alter the advert if the service already has characteristic data.
  virtual bool addCustomService(const BluetoothUUID& serviceId) = 0;

  /// \brief Removes the service (and all characteristics, if applicable) from the current Bluetooth advertisement. May cause advertising to restart.
  /// \since v2.1.0
  virtual void removeCustomService(const BluetoothUUID& serviceId) = 0;

  /// \brief Request to add a custom service advertisement. Returns false if it fails (E.g. exceeded advertisement data size). Provides a single characteristic and handlers. 
  /// \since v2.1.0
  /// Call multiple times for more than one characteristic on the same service.
  /// \note MUST guarantee that the same service is not advertised multiple times. Updates the advert if this service or characteristic is already advertised.
  virtual bool addCustomServiceCharacteristic(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const CharacteristicType& charType, const BLECallbacks& callbacks) = 0;

  /// \brief Removes the specified characteristic from the specified service definition in the Bluetooth advertisement. May cause advertising to restart.
  /// \since v2.1.0
  virtual void removeCustomServiceCharacteristic(const BluetoothUUID& serviceId, const BluetoothUUID& charId) = 0;

  /// \brief Notifies all subscribers that a characteristic value has changed. Fire and forget.
  /// \since v2.1.0
  virtual void notifyAllSubscribers(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const Data& newValue) = 0;

  /// \brief Notifies a specified subscriber that a characteristic value has changed. Fire and forget.
  /// \since v2.1.0
  virtual void notifySubscriber(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const Data& newValue, const BLEMacAddress& toNotify) = 0;
};

} // end namespace
} // end namespace

#endif