//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_concrete.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/bluetooth_state.h"

// C++17 includes
#include <memory>
#include <vector>

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
  std::shared_ptr<ConcreteBLETransmitter> transmitter;
  std::shared_ptr<ConcreteBLEReceiver> receiver;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;
};

ConcreteBLESensor::Impl::Impl(std::shared_ptr<Context> ctx, 
    std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
  : database(std::make_shared<ConcreteBLEDatabase>()), 
    transmitter(std::make_shared<ConcreteBLETransmitter>(
      ctx, bluetoothStateManager, payloadDataSupplier, database)
    ),
    receiver(std::make_shared<ConcreteBLEReceiver>(
      ctx, bluetoothStateManager, payloadDataSupplier, database)
    ),
    delegates()
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
  // bluetoothStateManager->add(shared_from_this()); // TEST FOR FAILURE IF USED HERE IN THE CTOR
  // mImpl->database->add(shared_from_this());
}

ConcreteBLESensor::~ConcreteBLESensor()
{
  ;
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
}

void
ConcreteBLESensor::start()
{
  mImpl->transmitter->start();
  mImpl->receiver->start();
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, SensorState::on);
  }
}

void
ConcreteBLESensor::stop()
{
  mImpl->transmitter->stop();
  mImpl->receiver->stop();
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, SensorState::off);
  }
}

// Database overrides
void
ConcreteBLESensor::bleDatabaseDidCreate(const std::shared_ptr<BLEDevice>& device)
{
  for (auto delegate : mImpl->delegates) {
    delegate->sensor(SensorType::BLE, device->identifier());
  }
}

void
ConcreteBLESensor::bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice>& device, 
  const BLEDeviceAttribute attribute)
{
  switch (attribute) {
    case BLEDeviceAttribute::rssi: {
      break;
    }
    case BLEDeviceAttribute::payloadData: {
      break;
    }
    default: {
      ; // do nothing
    }
  }
}

void
ConcreteBLESensor::bleDatabaseDidDelete(const std::shared_ptr<BLEDevice>& device)
{
  ; // TODO just log this // TODO determine if to pass this on too
}

// Bluetooth state manager delegate overrides
void
ConcreteBLESensor::bluetoothStateManager(BluetoothState didUpdateState)
{
  if (BluetoothState::poweredOff == didUpdateState) {
    stop();
  }
  if (BluetoothState::poweredOn == didUpdateState) {
    start();
  }
  if (BluetoothState::unsupported == didUpdateState) {
    for (auto delegate : mImpl->delegates) {
      delegate->sensor(SensorType::BLE, SensorState::unavailable);
    }
  }
}

}
}