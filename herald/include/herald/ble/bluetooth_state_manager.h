//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLUETOOTH_STATE_MANAGER_H
#define HERALD_BLUETOOTH_STATE_MANAGER_H

#include "ble.h"
#include "../datatype/bluetooth_state.h"
#include "../datatype/data.h"
#include "../datatype/proximity.h"
#include "ble_mac_address.h"
#include "bluetooth_state_manager_delegate.h"

namespace herald {
namespace ble {

using namespace herald::datatype;





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

  // TODO consider changing the below to use the full Service and Characteristic classes (may prevent errors - e.g. service and char uuid order reversal in calls)

  /// \brief Request to add a custom service advertisement. Returns false if it fails (E.g. exceeded advertisement data size). Provides a single characteristic and handlers. 
  /// \since v2.1.0
  /// Call multiple times for more than one characteristic on the same service.
  /// \note MUST guarantee that the same service is not advertised multiple times. Updates the advert if this service or characteristic is already advertised.
  virtual bool addCustomServiceCharacteristic(const BluetoothUUID& serviceId, const BluetoothUUID& charId, const BLECharacteristicType& charType, const BLECallbacks& callbacks) = 0;

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