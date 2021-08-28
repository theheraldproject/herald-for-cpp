//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BLE_CONCRETE_TRANSMITTER_H
#define HERALD_BLE_CONCRETE_TRANSMITTER_H

#include "../ble_database.h"
#include "../ble_receiver.h"
#include "../ble_sensor.h"
#include "../ble_transmitter.h"
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

// nRF Connect SDK includes
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

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


// zephyr internal functions called by template


namespace zephyrinternal {

  typedef std::function<PayloadData(const PayloadTimestamp)> GetPayloadFunction;
  
  GetPayloadFunction getPayloadDataSupplier();

  void setPayloadDataSupplier(GetPayloadFunction pds);

  
  struct bt_data* getAdvertData();
  std::size_t getAdvertDataSize();

  struct bt_le_adv_param* getAdvertParams();

  void get_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl);

  ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset);
  ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
    const void *buf, uint16_t len, uint16_t offset,
    uint8_t flags);
  ssize_t read_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset);
  ssize_t write_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
    const void *buf, uint16_t len, uint16_t offset,
    uint8_t flags);
}



template <typename ContextT, typename PayloadDataSupplierT, typename BLEDatabaseT, typename SensorDelegateSetT>
class ConcreteBLETransmitter {
public:
  ConcreteBLETransmitter(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
    PayloadDataSupplierT& payloadDataSupplier, BLEDatabaseT& bleDatabase, SensorDelegateSetT& dels)
    : m_context(ctx),
      m_stateManager(bluetoothStateManager),
      m_pds(payloadDataSupplier),
      m_db(bleDatabase),
      delegates(dels),
      isAdvertising(false)

    HLOGGERINIT(ctx,"Sensor","BLE.ConcreteBLETransmitter")
  {
    zephyrinternal::setPayloadDataSupplier([this](const PayloadTimestamp pts) -> PayloadData {
      return m_pds.payload(pts);
    });
  }

  ConcreteBLETransmitter(const ConcreteBLETransmitter& from) = delete;
  ConcreteBLETransmitter(ConcreteBLETransmitter&& from) = delete;

  ~ConcreteBLETransmitter()
  {
    stop();
    // zephyrinternal::setPayloadDataSupplier(NULL);
  }

  // Coordination overrides - Since v1.2-beta3
  std::optional<std::reference_wrapper<CoordinationProvider>> coordinationProvider() {
    return {};
  }

  // Sensor overrides
  void start() {
    HTDBG("ConcreteBLETransmitter::start");
    if (!m_context.getSensorConfiguration().advertisingEnabled) {
      HTDBG("Sensor Configuration has advertising disabled. Returning.");
      return;
    }
    m_context.getPlatform().getAdvertiser().registerStopCallback([this] () -> void {
      stopAdvertising();
    });
    m_context.getPlatform().getAdvertiser().registerStartCallback([this] () -> void {
      startAdvertising();
    });
    HTDBG("Advertising callbacks registered");

    // Ensure our zephyr context has bluetooth ready
    m_context.getPlatform().startBluetooth();

    HTDBG("Bluetooth started. Requesting start of adverts");

    startAdvertising();
  }

  void stop() {
    HTDBG("ConcreteBLETransmitter::stop");
    if (!m_context.getSensorConfiguration().advertisingEnabled) {
      HTDBG("Sensor Configuration has advertising disabled. Returning.");
      return;
    }
    stopAdvertising();
  }

private:
  ContextT& m_context;
  BluetoothStateManager& m_stateManager;
  PayloadDataSupplierT& m_pds;
  BLEDatabaseT& m_db;

  SensorDelegateSetT& delegates;

  bool isAdvertising;

  HLOGGER(ContextT);

  // Internal methods
  void startAdvertising()
  {
    // HTDBG("startAdvertising called");
    if (!m_context.getSensorConfiguration().advertisingEnabled) {
      HTDBG("Sensor Configuration has advertising disabled. Returning.");
      return;
    }
    if (isAdvertising) {
      // HTDBG("Already advertising. Returning.");
      return;
    }

    // Note: TxPower currently disabled due to restricted advert space and the need to include Herald's service. 
    //       See https://github.com/theheraldproject/herald-for-cpp/issues/26
    // Get current TxPower and alter advert accordingly:-
    // int8_t txp_get = 0;
    // zephyrinternal::get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,0, &txp_get);
    // HTDBG("Zephyr tx power:-");
    // HTDBG(std::to_string(txp_get));
    // herald::ble::ad[1] = bt_data{
    //   .type=BT_DATA_TX_POWER,
    //   .data_len=sizeof(txp_get),
    //   .data=(const uint8_t *)uint8_t(txp_get)
    // };

    // Now start advertising
    // See https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/bluetooth/gap.html#group__bt__gap_1gac45d16bfe21c3c38e834c293e5ebc42b
    int success = bt_le_adv_start(zephyrinternal::getAdvertParams(), zephyrinternal::getAdvertData(), zephyrinternal::getAdvertDataSize(), NULL, 0);
    if (0 != success) {
      HTDBG("Start advertising failed");
      return;
    }
    
    // zephyrinternal::get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,0, &txp_get);
    // HTDBG("Zephyr tx power post advertising starting:-");
    // HTDBG(std::to_string(txp_get));

    HTDBG("Start advertising completed successfully");
    isAdvertising = true;
  }

  void stopAdvertising()
  {
    // HTDBG("stopAdvertising called");
    if (!m_context.getSensorConfiguration().advertisingEnabled) {
      HTDBG("Sensor Configuration has advertising disabled. Returning.");
      return;
    }
    if (!isAdvertising) {
      // HTDBG("Not advertising already. Returning.");
      return;
    }
    isAdvertising = false;
    int success = bt_le_adv_stop();
    if (0 != success) {
      HTDBG("Stop advertising failed");
      return;
    }
    // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
    HTDBG("Stop advertising completed successfully");
  }

};

}
}

#endif