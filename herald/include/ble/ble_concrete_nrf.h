//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_CONCRETE_NRF_H
#define BLE_CONCRETE_NRF_H

#include "ble_concrete.h"
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

class ConcreteBLETransmitterNRF : public BLETransmitter {
public:
  ConcreteBLETransmitterNRF(int bleMacRotationSeconds,std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ~ConcreteBLETransmitterNRF();

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};


}
}

#endif
