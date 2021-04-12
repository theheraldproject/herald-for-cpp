//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_DEFAULT_BLE_CONCRETE_TRANSMITTER_H
#define HERALD_DEFAULT_BLE_CONCRETE_TRANSMITTER_H

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

// C++17 includes
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstring>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::ble::filter;
using namespace herald::payload;


/// \brief Dummy implementation of a ConcreteBLETransmitter that does nothing (used for testing)
template <typename ContextT, typename BLEDatabaseT>
class ConcreteBLETransmitter : public BLETransmitter {
public:
  ConcreteBLETransmitter(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, BLEDatabaseT& bleDatabase) {}

  ConcreteBLETransmitter(const ConcreteBLETransmitter& from) = delete;
  ConcreteBLETransmitter(ConcreteBLETransmitter&& from) = delete;

  ~ConcreteBLETransmitter() {}

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() override {
    return {};
  }

  // Sensor overrides
  void add(const std::shared_ptr<SensorDelegate>& delegate) override {}

  void start() override {}

  void stop() override {}

};

}
}

#endif