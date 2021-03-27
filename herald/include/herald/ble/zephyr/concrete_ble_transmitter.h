//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_TRANSMITTER_H
#define HERALD_BLE_CONCRETE_TRANSMITTER_H

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

// TODO zephyr internal functions called by template

template <typename ContextT>
class ConcreteBLETransmitter : public BLETransmitter, public std::enable_shared_from_this<ConcreteBLETransmitter<ContextT>> {
public:
  ConcreteBLETransmitter(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ConcreteBLETransmitter(const ConcreteBLETransmitter& from) = delete;
  ConcreteBLETransmitter(ConcreteBLETransmitter&& from) = delete;
  ~ConcreteBLETransmitter();

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::shared_ptr<CoordinationProvider>> coordinationProvider() override;

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override;
  void start() override;
  void stop() override;

private:
  class Impl;
  std::shared_ptr<Impl> mImpl; // shared to allow static callbacks to be bound
};

}
}

#endif