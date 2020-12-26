//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "context.h"
#include "ble/ble_concrete.h"
#include "ble/ble_database.h"
#include "ble/ble_receiver.h"
#include "ble/ble_sensor.h"
#include "ble/ble_transmitter.h"
#include "ble/bluetooth_state_manager.h"

// nRF Connect SDK includes
#include <bluetooth.h>

// C++17 includes
#include <memory>
#include <vector>

namespace herald {
namespace ble {

using namespace herald::datatype;

ConcreteBLETranmsmitterNRF::ConcreteBLETranmsmitterNRF(int bleMacRotationSeconds,
  std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : m_bleRotation(bleMacRotationSeconds),
    m_addr(new uint8_t(0));
{

}

ConcreteBLETranmsmitterNRF::~ConcreteBLETransmitterNRF()
{
  stop(); // stops using m_addr
  delete m_addr; // deletes m_addr (created by ctor)
}

void
ConcreteBLETranmsmitterNRF::start()
{
  // rotate mac address using onboard crypto
  auto success = bt_rand(&m_addr, 1);
  if (!success) {
    // TODO log
    return;
  }
  // TODO set rotation timer, if set (not -1)
  success = bt_set_id_addr(&m_addr);
  if (!success) {
    // TODO log
    return;
  }
  success = bt_ctlr_set_public_addr(&m_addr); // TODO figure out if both are required by NRF
  if (!success) {
    // TODO log
    return;
  }
  success = bt_set_name("heraldbeacon"); // TODO vary this by application
  if (!success) {
    // TODO log
    return;
  }
  success = bt_enable(NULL); // NULL means synchronously enabled
  // TODO consider abstracting BLE status out to an NRF BLE specific class

  // Now start advertising
  // See https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/bluetooth/gap.html#group__bt__gap_1gac45d16bfe21c3c38e834c293e5ebc42b
  bt_le_adv_param param{}; // TODO initialise parameters
  bt_data ad{};
  size_t ad_len;
  bt_data sd{}; // service definition
  size_t sd_len; // service definition length
  success = bt_le_adv_start(&param, &ad, ad_len, &sd, sd_len);
  if (!success) {
    // TODO log
    return;
  }
}

void
ConcreteBLETranmsmitterNRF::stop()
{
  int success = bt_le_adv_stop();
  if (!success) {
    // TODO log
    return;
  }
}