//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_CONCRETE_H
#define BLE_CONCRETE_H

#include "ble_database.h"
#include "ble_receiver.h"
#include "ble_sensor.h"
#include "ble_transmitter.h"
#include "bluetooth_state_manager.h"
#include "../payload/payload_data_supplier.h"
#include "../context.h"

#include <memory>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::payload;

// NOTE THIS HEADER IS FOR ALL PLATFORMS. 
//      SPECIFIC PLATFORM DEFINITIONS ARE WITHIN SEVERAL C++ FILES
//      UNDER WINDOWS AND ZEPHYR SUB DIRECTORIES

class ConcreteBLEDatabase : public BLEDatabase, public BLEDeviceDelegate {
public:
  ConcreteBLEDatabase();
  ~ConcreteBLEDatabase();

  // TODO add solid (non virtual) methods to be implemented by this class

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

// class ConcreteBLESensor : public BLESensor {
// public:
//   ConcreteBLESensor(std::shared_ptr<Context> ctx, std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
//   ~ConcreteBLESensor();

//   bool immediateSend(Data data, const TargetIdentifier& targetIdentifier);

//   // overrides
//   void add(std::shared_ptr<SensorDelegate> delegate) override;
//   void start() override;
//   void stop() override;

// private:
//   class Impl;
//   std::unique_ptr<Impl> mImpl;
// };

// class ConcreteBLEReceiver : public BLEReceiver {
// public:
//   ConcreteBLEReceiver(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
//     std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase,
//     std::shared_ptr<BLETransmitter> bleTransmitter);
//   ~ConcreteBLEReceiver();

//   bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override;

// private:
//   class Impl;
//   std::unique_ptr<Impl> mImpl;
// };

class ConcreteBLETransmitter : public BLETransmitter {
public:
  ConcreteBLETransmitter(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ~ConcreteBLETransmitter();

  // Sensor overrides
  void add(std::shared_ptr<SensorDelegate> delegate) override;
  void start() override;
  void stop() override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

// class ConcreteBluetoothStateManager : public BluetoothStateManager {
// public:
//   ConcreteBluetoothStateManager(std::shared_ptr<Context> ctx);
//   ~ConcreteBluetoothStateManager();

// private:
//   class Impl;
//   std::unique_ptr<Impl> mImpl;
// };

} // end namespace
} // end namespace

#endif