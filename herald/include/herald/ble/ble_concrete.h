//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_H
#define HERALD_BLE_CONCRETE_H

#include "ble_concrete_database.h"
#include "ble_database.h"
#include "ble_receiver.h"
#include "ble_sensor.h"
#include "ble_transmitter.h"
#include "ble_concrete.h"
#include "ble_protocols.h"
#include "bluetooth_state_manager.h"
#include "ble_device_delegate.h"
#include "filter/ble_advert_parser.h"
#include "../payload/payload_data_supplier.h"
#include "../context.h"
#include "../data/sensor_logger.h"
#include "ble_sensor_configuration.h"
#include "ble_coordinator.h"
#include "../datatype/bluetooth_state.h"

// Include the relevant concrete BLE Receiver here
#ifdef __ZEPHYR__
#ifdef CONFIG_BT_SCAN
#include "zephyr/concrete_ble_receiver.h"
#else
#include "default/concrete_ble_receiver.h"
#endif
#include "zephyr/concrete_ble_transmitter.h"
// TODO other platforms here
#else
#include "default/concrete_ble_receiver.h"
#include "default/concrete_ble_transmitter.h"
#endif

#include <memory>
#include <vector>
#include <algorithm>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;

// NOTE THIS HEADER IS FOR ALL PLATFORMS. 
//      SPECIFIC PLATFORM DEFINITIONS ARE WITHIN SEVERAL C++ FILES
//      UNDER WINDOWS AND ZEPHYR SUB DIRECTORIES


/**
 * Acts as the main object to control the receiver, transmitter, and database instances
 */
template <typename ContextT, typename PayloadDataSupplierT, typename SensorDelegateSetT, std::size_t DBSize = 10>
class ConcreteBLESensor : public BLEDatabaseDelegate, 
  public BluetoothStateManagerDelegate {
public:
  ConcreteBLESensor(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    PayloadDataSupplierT& payloadDataSupplier, SensorDelegateSetT& dels)
  : m_context(ctx),
    database(ctx), 
    stateManager(bluetoothStateManager),
    transmitter(ctx, bluetoothStateManager, payloadDataSupplier, database, dels),
    receiver(ctx, bluetoothStateManager, payloadDataSupplier, database, dels),
    delegates(dels),
    coordinator(ctx, database, receiver),
    addedSelfAsDelegate(false)
    HLOGGERINIT(ctx,"sensor","ConcreteBLESensor")
  {
  }

  // Delete for GCC 8/9. See https://stackoverflow.com/questions/63812165/stdvariant-requires-default-constructor-in-gcc-8-and-9-and-not-require-in-gcc
  ConcreteBLESensor() = delete;
  ConcreteBLESensor(const ConcreteBLESensor& from) = delete;
  ConcreteBLESensor(ConcreteBLESensor&& from) = delete;
  ~ConcreteBLESensor() = default;

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() {
    // Only return this if we support scanning
    if (m_context.getSensorConfiguration().scanningEnabled) {
      HTDBG("Providing a BLECoordinationProvider");
      //return std::optional<std::reference_wrapper<CoordinationProvider>>(std::static_cast<CoordinationProvider>(coordinator));
      return coordinator;
    }
    HTDBG("Scanning not supported - so not returning a BLECoordinationProvider");
    return {};
  }

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) {
    return receiver.immediateSend(data,targetIdentifier);
  }

  bool immediateSendAll(Data data) {
    return receiver.immediateSendAll(data);
  }

  // Sensor overrides
  void start() {
    if (!addedSelfAsDelegate) {
      stateManager.add(*this); // FAILS IF USED IN THE CTOR - DO NOT DO THIS FROM CTOR
      database.add(*this);
      addedSelfAsDelegate = true;
    }
    transmitter.start();
    receiver.start();
    // for (auto& delegate : delegates) {
      delegates.sensor(SensorType::BLE, SensorState::on);
    // }
  }

  void stop() {
    transmitter.stop();
    receiver.stop();
    // for (auto& delegate : delegates) {
      delegates.sensor(SensorType::BLE, SensorState::off);
    // }
  }

  // Database overrides
  void bleDatabaseDidCreate(const BLEDevice& device) override {
    // for (auto& delegate : delegates) {
      delegates.sensor(SensorType::BLE, device.identifier()); // didDetect
    // }
  }

  void bleDatabaseDidUpdate(const BLEDevice& device, const BLEDeviceAttribute attribute) override {
    switch (attribute) {
      case BLEDeviceAttribute::rssi: {
        auto rssi = device.rssi();
        if (rssi.intValue() != 0) {
          double rssiValue = (double)rssi.intValue();
          auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
          // for (auto& delegate: delegates) {
            delegates.sensor(SensorType::BLE,
              prox,
              device.identifier()
            ); // didMeasure
          // }
          // also payload with rssi
          auto payload = device.payloadData();
          if (payload.size() > 0) {
            // for (auto& delegate: delegates) {
              delegates.sensor(SensorType::BLE,
                prox,
                device.identifier(),
                payload
              ); // didMeasure withPayload
            // }
          }
        }
        break;
      }
      case BLEDeviceAttribute::payloadData: {
        auto payload = device.payloadData();
        if (payload.size() > 0) {
          // for (auto& delegate: delegates) {
            delegates.sensor(SensorType::BLE,
              payload,
              device.identifier()
            ); // didReadPayload
          // }
          // also payload with rssi
          auto rssi = device.rssi();
          if (rssi.intValue() != 0) {
            double rssiValue = (double)rssi.intValue();
            auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
            // for (auto& delegate: delegates) {
              delegates.sensor(SensorType::BLE,
                prox,
                device.identifier(),
                payload
              ); // didMeasure withPayload
            // }
          }
        }
        break;
      }
      default: {
        ; // do nothing
      }
    }
  }

  void bleDatabaseDidDelete(const BLEDevice& device) override {
    ; // TODO just log this // TODO determine if to pass this on too
    // TODO fire this for analysis runner and others' benefit
  }

  // Bluetooth state manager delegate overrides
  void bluetoothStateManager(BluetoothState didUpdateState) override {
    if (BluetoothState::poweredOff == didUpdateState) {
      // stop();
    }
    if (BluetoothState::poweredOn == didUpdateState) {
      // start();
    }
    if (BluetoothState::unsupported == didUpdateState) {
      // for (auto& delegate : delegates) {
        delegates.sensor(SensorType::BLE, SensorState::unavailable);
      // }
    }
  }

private:

  // Internal API private methods here too

  // Data members hidden by PIMPL

  ContextT& m_context;
  ConcreteBLEDatabase<ContextT,DBSize> database;
  BluetoothStateManager& stateManager;
  ConcreteBLETransmitter<ContextT,PayloadDataSupplierT,ConcreteBLEDatabase<ContextT,DBSize>,SensorDelegateSetT> transmitter;
  ConcreteBLEReceiver<ContextT,PayloadDataSupplierT,ConcreteBLEDatabase<ContextT,DBSize>,SensorDelegateSetT> receiver;

  SensorDelegateSetT& delegates;
  
  HeraldProtocolBLECoordinationProvider<
    ContextT,
    ConcreteBLEDatabase<ContextT,DBSize>,
    ConcreteBLEReceiver<ContextT,PayloadDataSupplierT,ConcreteBLEDatabase<ContextT,DBSize>,SensorDelegateSetT>
  > coordinator;

  bool addedSelfAsDelegate;

  HLOGGER(ContextT);
};

} // end namespace
} // end namespace

#endif