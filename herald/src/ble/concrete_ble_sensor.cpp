//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_concrete.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/ble/ble_coordinator.h"
#include "herald/ble/ble_sensor_configuration.h"
#include "herald/data/sensor_logger.h"

// C++17 includes
#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

class ConcreteBLESensor::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, 
    std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ~Impl();

  // TODO internal API private methods here too

  // Data members hidden by PIMPL

  std::shared_ptr<ConcreteBLEDatabase> database;
  std::shared_ptr<BluetoothStateManager> stateManager;
  std::shared_ptr<ConcreteBLETransmitter> transmitter;
  std::shared_ptr<ConcreteBLEReceiver> receiver;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;
  
  std::shared_ptr<HeraldProtocolBLECoordinationProvider> coordinator;

  bool addedSelfAsDelegate;

  HLOGGER;
};

ConcreteBLESensor::Impl::Impl(std::shared_ptr<Context> ctx, 
    std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : database(std::make_shared<ConcreteBLEDatabase>(ctx)), 
    stateManager(bluetoothStateManager),
    transmitter(std::make_shared<ConcreteBLETransmitter>(
      ctx, bluetoothStateManager, payloadDataSupplier, database)
    ),
    receiver(std::make_shared<ConcreteBLEReceiver>(
      ctx, bluetoothStateManager, payloadDataSupplier, database)
    ),
    delegates(),
    coordinator(std::make_shared<HeraldProtocolBLECoordinationProvider>(ctx, database, receiver)),
    addedSelfAsDelegate(false)
    HLOGGERINIT(ctx,"sensor","ConcreteBLESensor")
{
  ;
}

ConcreteBLESensor::Impl::~Impl()
{
  ;
}






ConcreteBLESensor::ConcreteBLESensor(std::shared_ptr<Context> ctx, 
    std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : mImpl(std::make_unique<Impl>(ctx,bluetoothStateManager,payloadDataSupplier))
{
  // Note: Use of shared_from_this is safe as we known SensorArray 
  //       creates a shared_ptr of this class during instantiation
  // bluetoothStateManager->add(shared_from_this()); // TEST FOR FAILURE IF USED HERE IN THE CTOR - YES IT FAILS, DO NOT DO THIS FROM CTOR
  // mImpl->database->add(shared_from_this());
}

ConcreteBLESensor::~ConcreteBLESensor()
{
  ;
}

std::optional<std::shared_ptr<CoordinationProvider>>
ConcreteBLESensor::coordinationProvider()
{
  // Only return this if we support scanning
  if (BLESensorConfiguration::scanningEnabled) {
    HDBG("Providing a BLECoordinationProvider");
    return std::optional<std::shared_ptr<CoordinationProvider>>(mImpl->coordinator);
  }
  HDBG("Scanning not supported - so not returning a BLECoordinationProvider");
  return {};
}

bool
ConcreteBLESensor::immediateSend(Data data, const TargetIdentifier& targetIdentifier)
{
  return mImpl->receiver->immediateSend(data,targetIdentifier);
}

bool
ConcreteBLESensor::immediateSendAll(Data data)
{
  return mImpl->receiver->immediateSendAll(data);
}

// Sensor overrides
void
ConcreteBLESensor::add(std::shared_ptr<SensorDelegate> delegate)
{
  mImpl->delegates.push_back(delegate);
  // add all delegates to receiver and transmitter too?
  mImpl->receiver->add(delegate);
  mImpl->transmitter->add(delegate);
  // TODO what about duplicates?
}

void
ConcreteBLESensor::start()
{
  if (!mImpl->addedSelfAsDelegate) {
    mImpl->stateManager->add(shared_from_this()); // FAILS IF USED IN THE CTOR - DO NOT DO THIS FROM CTOR
    mImpl->database->add(shared_from_this());
    mImpl->addedSelfAsDelegate = true;
  }
  // mImpl->transmitter->start();
  mImpl->receiver->start();
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, SensorState::on);
  }
}

void
ConcreteBLESensor::stop()
{
  // mImpl->transmitter->stop();
  mImpl->receiver->stop();
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, SensorState::off);
  }
}

// Database overrides
void
ConcreteBLESensor::bleDatabaseDidCreate(const std::shared_ptr<BLEDevice> device)
{
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, device->identifier()); // didDetect
  }
}

void
ConcreteBLESensor::bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice> device, 
  const BLEDeviceAttribute attribute)
{
  switch (attribute) {
    case BLEDeviceAttribute::rssi: {
      auto rssi = device->rssi();
      if (rssi.has_value()) {
        double rssiValue = (double)rssi->intValue();
        auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
        for (auto delegate: mImpl->delegates) {
          delegate->sensor(SensorType::BLE,
            prox,
            device->identifier()
          ); // didMeasure
        }
        // also payload with rssi
        auto payload = device->payloadData();
        if (payload.has_value()) {
          for (auto delegate: mImpl->delegates) {
            delegate->sensor(SensorType::BLE,
              prox,
              device->identifier(),
              *payload
            ); // didReadPayloadAndMeasure
          }
        }
      }
      break;
    }
    case BLEDeviceAttribute::payloadData: {
      auto payload = device->payloadData();
      if (payload.has_value()) {
        for (auto delegate: mImpl->delegates) {
          delegate->sensor(SensorType::BLE,
            *payload,
            device->identifier()
          ); // didReadPayload
        }
        // also payload with rssi
        auto rssi = device->rssi();
        if (rssi.has_value()) {
          double rssiValue = (double)rssi->intValue();
          auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
          for (auto delegate: mImpl->delegates) {
            delegate->sensor(SensorType::BLE,
              prox,
              device->identifier(),
              *payload
            ); // didReadPayloadAndMeasure
          }
        }
      }
      break;
    }
    default: {
      ; // do nothing
    }
  }
}

void
ConcreteBLESensor::bleDatabaseDidDelete(const std::shared_ptr<BLEDevice> device)
{
  ; // TODO just log this // TODO determine if to pass this on too
}

// Bluetooth state manager delegate overrides
void
ConcreteBLESensor::bluetoothStateManager(BluetoothState didUpdateState)
{
  if (BluetoothState::poweredOff == didUpdateState) {
    // stop();
  }
  if (BluetoothState::poweredOn == didUpdateState) {
    // start();
  }
  if (BluetoothState::unsupported == didUpdateState) {
    for (auto delegate : mImpl->delegates) {
      delegate->sensor(SensorType::BLE, SensorState::unavailable);
    }
  }
}

}
}