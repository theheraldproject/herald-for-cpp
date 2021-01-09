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
#include "ble_device_delegate.h"
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

class ConcreteBLEDatabase : public BLEDatabase, public BLEDeviceDelegate, public std::enable_shared_from_this<ConcreteBLEDatabase>  {
public:
  ConcreteBLEDatabase();
  ConcreteBLEDatabase(const ConcreteBLEDatabase& from) = delete;
  ConcreteBLEDatabase(ConcreteBLEDatabase&& from) = delete;
  ~ConcreteBLEDatabase();

  // BLE Database overrides

  void add(const std::shared_ptr<BLEDatabaseDelegate>& delegate) override;

  // Creation overrides
  std::shared_ptr<BLEDevice> device(const PayloadData& payloadData) override;

  std::shared_ptr<BLEDevice> device(const TargetIdentifier& targetIdentifier) override;
  
  // Introspection overrides
  std::size_t size() const override;

  std::vector<std::shared_ptr<BLEDevice>> matches(
    const std::function<bool(std::shared_ptr<BLEDevice>)>& matcher) const override;

  // std::vector<std::shared_ptr<BLEDevice>> devices() const override;

  /// Cannot name a function delete in C++. remove is common.
  void remove(const TargetIdentifier& targetIdentifier) override;

  // std::optional<PayloadSharingData> payloadSharingData(const std::shared_ptr<BLEDevice>& peer) override;

  // BLE Device Delegate overrides
  void device(std::shared_ptr<BLEDevice> device, BLEDeviceAttribute didUpdate) override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // unique as this is handled internally for all platforms by Herald
};

/**
 * Acts as the main object to control the receiver, transmitter, and database instances
 */
class ConcreteBLESensor : public BLESensor, public BLEDatabaseDelegate, 
  public BluetoothStateManagerDelegate, public std::enable_shared_from_this<ConcreteBLESensor>  {
public:
  ConcreteBLESensor(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
  ConcreteBLESensor(const ConcreteBLESensor& from) = delete;
  ConcreteBLESensor(ConcreteBLESensor&& from) = delete;
  ~ConcreteBLESensor();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier);
  bool immediateSendAll(Data data);

  // Sensor overrides
  void add(std::shared_ptr<SensorDelegate> delegate) override;
  void start() override;
  void stop() override;

  // Database overrides
  void bleDatabaseDidCreate(const std::shared_ptr<BLEDevice> device) override;
  void bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice> device, const BLEDeviceAttribute attribute) override;
  void bleDatabaseDidDelete(const std::shared_ptr<BLEDevice> device) override;

  // Bluetooth state manager delegate overrides
  void bluetoothStateManager(BluetoothState didUpdateState) override;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // unique as this is handled internally for all platforms by Herald
};

class ConcreteBLEReceiver : public BLEReceiver, public std::enable_shared_from_this<ConcreteBLEReceiver> {
public:
  ConcreteBLEReceiver(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLEReceiver(const ConcreteBLEReceiver& from) = delete;
  ConcreteBLEReceiver(ConcreteBLEReceiver&& from) = delete;
  ~ConcreteBLEReceiver();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override;
  bool immediateSendAll(Data data) override;

  // Sensor overrides
  void add(std::shared_ptr<SensorDelegate> delegate) override;
  void start() override;
  void stop() override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

class ConcreteBLETransmitter : public BLETransmitter, public std::enable_shared_from_this<ConcreteBLETransmitter> {
public:
  ConcreteBLETransmitter(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLETransmitter(const ConcreteBLETransmitter& from) = delete;
  ConcreteBLETransmitter(ConcreteBLETransmitter&& from) = delete;
  ~ConcreteBLETransmitter();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  // Sensor overrides
  void add(std::shared_ptr<SensorDelegate> delegate) override;
  void start() override;
  void stop() override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

} // end namespace
} // end namespace

#endif