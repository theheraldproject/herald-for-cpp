//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_RECEIVER_H
#define HERALD_BLE_CONCRETE_RECEIVER_H

#include "../ble_database.h"
#include "../ble_receiver.h"
#include "../ble_sensor.h"
#include "../ble_transmitter.h"
#include "../ble_concrete.h"
#include "../ble_protocols.h"
#include "../bluetooth_state_manager.h"
#include "../ble_device_delegate.h"
#include "../filter/ble_advert_parser.h"
#include "../../payload/payload_data_supplier.h"
#include "../../context.h"
#include "../../data/sensor_logger.h"
#include "../ble_sensor_configuration.h"
#include "../ble_coordinator.h"
#include "../../datatype/bluetooth_state.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;

template <typename ContextT>
class ConcreteBLEReceiver : public BLEReceiver, public HeraldProtocolV1Provider, public std::enable_shared_from_this<ConcreteBLEReceiver<ContextT>> {
public:
  ConcreteBLEReceiver(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLEReceiver(const ConcreteBLEReceiver& from) = delete;
  ConcreteBLEReceiver(ConcreteBLEReceiver&& from) = delete;
  ~ConcreteBLEReceiver();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override;
  bool immediateSendAll(Data data) override;

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override;
  void start() override;
  void stop() override;

  // Herald V1 protocol provider overrides
  // C++17 CALLBACK VERSION:-
  // void openConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void closeConnection(const TargetIdentifier& toTarget, const HeraldConnectionCallback& connCallback) override;
  // void serviceDiscovery(Activity, CompletionCallback) override;
  // void readPayload(Activity, CompletionCallback) override;
  // void immediateSend(Activity, CompletionCallback) override;
  // void immediateSendAll(Activity, CompletionCallback) override;
  
  // NON C++17 VERSION:-
  bool openConnection(const TargetIdentifier& toTarget) override;
  bool closeConnection(const TargetIdentifier& toTarget) override;
  void restartScanningAndAdvertising() override;
  std::optional<Activity> serviceDiscovery(Activity) override;
  std::optional<Activity> readPayload(Activity) override;
  std::optional<Activity> immediateSend(Activity) override;
  std::optional<Activity> immediateSendAll(Activity) override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

}
}

#endif