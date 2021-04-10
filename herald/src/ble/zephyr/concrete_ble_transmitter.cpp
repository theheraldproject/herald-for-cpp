//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/zephyr/concrete_ble_transmitter.h"

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
#include <cstring>

namespace herald {
namespace ble {

using namespace herald;
using namespace herald::datatype;
using namespace herald::data;

namespace zephyrinternal {
  [[maybe_unused]]
  static void get_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl)
  {
    struct bt_hci_cp_vs_read_tx_power_level *cp;
    struct bt_hci_rp_vs_read_tx_power_level *rp;
    struct net_buf *buf, *rsp = NULL;
    int err;

    *tx_pwr_lvl = 0xFF;
    buf = bt_hci_cmd_create(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
          sizeof(*cp));
    if (!buf) {
      printk("Unable to allocate command buffer\n");
      return;
    }

    cp = (bt_hci_cp_vs_read_tx_power_level*)net_buf_add(buf, sizeof(*cp));
    cp->handle = sys_cpu_to_le16(handle);
    cp->handle_type = handle_type;

    err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
            buf, &rsp);
    if (err) {
      uint8_t reason = rsp ?
        ((struct bt_hci_rp_vs_read_tx_power_level *)
          rsp->data)->status : 0;
      printk("Read Tx power err: %d reason 0x%02x\n", err, reason);
      return;
    }

    rp = (bt_hci_rp_vs_read_tx_power_level *)rsp->data;
    *tx_pwr_lvl = rp->tx_power_level;

    net_buf_unref(rsp);
  }

  // template <typename ContextT>
  // class ConcreteBLETransmitter<ContextT>::Impl {
  // public:
  //   Impl(ContextT& ctx, BluetoothStateManager& bluetoothStateManager, 
  //     std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  //   ~Impl();

  //   void startAdvertising();
  //   void stopAdvertising();

  //   ContextT& m_context;
  //   BluetoothStateManager& m_stateManager;
  //   std::shared_ptr<PayloadDataSupplier> m_pds;
  //   std::shared_ptr<BLEDatabase> m_db;

  //   std::vector<std::shared_ptr<SensorDelegate>> delegates;

  //   bool isAdvertising;

  //   HLOGGER(ContextT);
  // };


  // TODO bind the below to a class and control their values

  static uint8_t vnd_value[] = { 'V', 'e', 'n', 'd', 'o', 'r' };

  static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
        void *buf, uint16_t len, uint16_t offset)
  {
    const char *value = (const char*)attr->user_data;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
          strlen(value));
  }

  static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
        const void *buf, uint16_t len, uint16_t offset,
        uint8_t flags)
  {
    uint8_t *value = (uint8_t*)attr->user_data;

    if (offset + len > sizeof(vnd_value)) {
      return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    return len;
  }

  static ssize_t read_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
        void *buf, uint16_t len, uint16_t offset)
  {
    const char *value = (const char*)attr->user_data;
    if (NULL != latestPds) {
      PayloadTimestamp pts; // now
      auto payload = latestPds->payload(pts,nullptr);
      if (payload.has_value()) {
        char* newvalue = new char[payload->size()];
        std::size_t i;
        for (i = 0;i < payload->size();i++) {
          newvalue[i] = (char)payload->at(i);
        }
        value = newvalue;
        return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
          payload->size());
      // } else {
      //   value = "venue value"; // TODO replace with the use of PDS
      }
    }
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
      strlen(value));
  }

  static ssize_t write_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
        const void *buf, uint16_t len, uint16_t offset,
        uint8_t flags)
  {
    uint8_t *value = (uint8_t*)attr->user_data;

    if (offset + len > sizeof(vnd_value)) {
      return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    return len;
  }

}

}
}
