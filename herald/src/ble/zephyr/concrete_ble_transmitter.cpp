//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/zephyr/concrete_ble_transmitter.h"
#include "herald/payload/payload_data_supplier.h"

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
using namespace herald::payload;

namespace zephyrinternal {
  // FWD DECLS from concrete_ble_receiver
  // struct bt_uuid_128* getHeraldUUID();
  // struct bt_uuid_128* getHeraldSignalAndroidCharUUID();
  // struct bt_uuid_128* getHeraldPayloadCharUUID();

  /* Herald Service Variables */
  // struct bt_uuid_128 herald_uuid = BT_UUID_INIT_128(
  //   0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
  // );
  // struct bt_uuid_128 herald_char_signal_uuid = BT_UUID_INIT_128(
  //   0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
  // );
  // struct bt_uuid_128 herald_char_payload_uuid = BT_UUID_INIT_128(
  //   0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
  // );
  // Define kernel memory statically so we definitely have it
  // struct bt_gatt_attr chrc_signal[2] = BT_GATT_CHARACTERISTIC(&zephyrinternal::getHeraldSignalAndroidCharUUID()->uuid,
  //             BT_GATT_CHRC_WRITE,
  //             BT_GATT_PERM_WRITE,
  //             zephyrinternal::read_vnd,zephyrinternal::write_vnd, nullptr);
  // auto chrc_payload = BT_GATT_CHARACTERISTIC(&zephyrinternal::getHeraldPayloadCharUUID()->uuid,
  //             BT_GATT_CHRC_READ,
  //             BT_GATT_PERM_READ,
  //             zephyrinternal::read_payload, zephyrinternal::write_payload, nullptr);
  // struct bt_gatt_attr attr_herald_svc_name[] = {
  //   BT_GATT_PRIMARY_SERVICE(getHeraldUUID()),
  //   chrc_signal,
  //   chrc_payload
  // };
  // // const Z_STRUCT_SECTION_ITERABLE(bt_gatt_service_static, herald_svc) =
	// // 					BT_GATT_SERVICE(attr_herald_svc_name);
	// Z_DECL_ALIGN(struct bt_gatt_service_static) herald_svc __in_section(_bt_gatt_service_static, static, herald_svc) __used;

  struct bt_uuid_128 herald_uuid_tx = BT_UUID_INIT_128(
    0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
  );
  struct bt_uuid_128 herald_char_signal_android_uuid_tx = BT_UUID_INIT_128(
    0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
  );
  struct bt_uuid_128 herald_char_signal_ios_uuid_tx = BT_UUID_INIT_128(
    0x63, 0x43, 0x2d, 0xb0, 0xad, 0xa4, 0xf3, 0x8a, 0x9a, 0x4a, 0xe4, 0xea, 0xf2, 0xd5, 0xb0, 0x0e
  );
  struct bt_uuid_128 herald_char_payload_uuid_tx = BT_UUID_INIT_128(
    0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
  );
  BT_GATT_SERVICE_DEFINE(herald_svc,
    BT_GATT_PRIMARY_SERVICE(&herald_uuid_tx),
    BT_GATT_CHARACTERISTIC(&herald_char_signal_android_uuid_tx.uuid,
              BT_GATT_CHRC_WRITE,
              BT_GATT_PERM_WRITE,
              zephyrinternal::read_vnd,zephyrinternal::write_vnd, nullptr),
    BT_GATT_CHARACTERISTIC(&herald_char_payload_uuid_tx.uuid,
              BT_GATT_CHRC_READ,
              BT_GATT_PERM_READ,
              zephyrinternal::read_payload, zephyrinternal::write_payload, nullptr)
  );


  auto bp = BT_LE_ADV_CONN_NAME; // No TxPower support
  /*
  static auto bp = BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
                BT_LE_ADV_OPT_USE_NAME, \
                BT_GAP_ADV_FAST_INT_MIN_2, \
                BT_GAP_ADV_FAST_INT_MAX_2, \
                BT_LE_ADV_OPT_USE_TX_POWER, \
                NULL); // txpower - REQUIRES EXT ADV OPT on Zephyr (experimental)
  */
  struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    // BT_DATA_BYTES(BT_DATA_TX_POWER, 0x00 ), // See https://github.com/theheraldproject/herald-for-cpp/issues/26
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 
            BT_UUID_16_ENCODE(BT_UUID_DIS_VAL),
            BT_UUID_16_ENCODE(BT_UUID_GATT_VAL),
            BT_UUID_16_ENCODE(BT_UUID_GAP_VAL)
    ),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,
            0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 
            0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
    ),
  };

  struct bt_data* getAdvertData() {
    return ad;
  }
  std::size_t getAdvertDataSize() {
    return ARRAY_SIZE(ad);
  }

  struct bt_le_adv_param* getAdvertParams() {
    return bp;
  }

  PayloadDataSupplier* latestPds = NULL;

  PayloadDataSupplier* getPayloadDataSupplier() {
    return latestPds;
  }

  void setPayloadDataSupplier(PayloadDataSupplier* pds) {
    latestPds = pds;
  }

  [[maybe_unused]]
  void get_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl)
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


  // TODO bind the below to a class and control their values - support for writing our payload to the remote device.

  uint8_t vnd_value[] = { 'V', 'e', 'n', 'd', 'o', 'r' };

  ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
        void *buf, uint16_t len, uint16_t offset)
  {
    const char *value = (const char*)attr->user_data;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
          strlen(value));
  }

  ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
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

  ssize_t read_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
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
        auto res = bt_gatt_attr_read(conn, attr, buf, len, offset, value,
          payload->size());
        delete newvalue;
        return res;
      // } else {
      //   value = "venue value"; // TODO replace with the use of PDS
      }
    }
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
      strlen(value));
  }

  ssize_t write_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
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
