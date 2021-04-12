//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEFAULT_BLE_CONCRETE_RECEIVER_H
#define HERALD_DEFAULT_BLE_CONCRETE_RECEIVER_H

#include "../bluetooth_state_manager.h"
#include "../ble_receiver.h"
#include "../../payload/payload_data_supplier.h"
#include "../../datatype/data.h"
#include "../../datatype/target_identifier.h"

#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::payload;

/// \brief Dummy implementation of a ConcreteBLEReceiver that does nothing (used for testing)
template <typename ContextT, typename BLEDatabaseT>
class ConcreteBLEReceiver : public BLEReceiver, public HeraldProtocolV1Provider {
public:
  ConcreteBLEReceiver(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, BLEDatabaseT& bleDatabase) {}
  ConcreteBLEReceiver(const ConcreteBLEReceiver& from) = delete;
  ConcreteBLEReceiver(ConcreteBLEReceiver&& from) = delete;
  ~ConcreteBLEReceiver() {}

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() override {
    return {};
  }

  bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) override {
    return false;
  }
  bool immediateSendAll(Data data) override {
    return false;
  }

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override {}
  void start() override {}
  void stop() override {}

  // Herald V1 Protocol Provider methods
  bool openConnection(const TargetIdentifier& toTarget) override {
    return false;
  }

  bool closeConnection(const TargetIdentifier& toTarget) override {
    return true;
  }

  void restartScanningAndAdvertising() override {
    ;
  }

  std::optional<Activity> serviceDiscovery(Activity) override {
    return {};
  }

  std::optional<Activity> readPayload(Activity) override {
    return {};
  }
  
  std::optional<Activity> immediateSend(Activity) override {
    return {};
  }
  
  std::optional<Activity> immediateSendAll(Activity) override {
    return {};
  }
  
};

}
}

#endif