//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/ble_sensor_configuration.h"
#include "herald/ble/ble_transmitter.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/data/sensor_logger.h"

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
#include <cstring>

namespace herald {
namespace ble {

using namespace herald;
using namespace herald::datatype;
using namespace herald::data;

static PayloadDataSupplier* latestPds = NULL;

namespace zephyrinternal {
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
}

class ConcreteBLETransmitter::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ~Impl();

  void startAdvertising();
  void stopAdvertising();

  std::shared_ptr<ZephyrContext> m_context;
  std::shared_ptr<BluetoothStateManager> m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  std::shared_ptr<BLEDatabase> m_db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  bool isAdvertising;

  HLOGGER;
};

/* Herald Service Variables */
static struct bt_uuid_128 herald_uuid = BT_UUID_INIT_128(
	0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
);
static struct bt_uuid_128 herald_char_signal_uuid = BT_UUID_INIT_128(
	0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
);
static struct bt_uuid_128 herald_char_payload_uuid = BT_UUID_INIT_128(
	0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
);

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

// Define kernel memory statically so we definitely have it
BT_GATT_SERVICE_DEFINE(herald_svc,
  BT_GATT_PRIMARY_SERVICE(&herald_uuid),
  BT_GATT_CHARACTERISTIC(&herald_char_signal_uuid.uuid,
            BT_GATT_CHRC_WRITE,
            BT_GATT_PERM_WRITE,
            read_vnd,write_vnd, nullptr),
  BT_GATT_CHARACTERISTIC(&herald_char_payload_uuid.uuid,
            BT_GATT_CHRC_READ,
            BT_GATT_PERM_READ,
            read_payload, write_payload, nullptr)
);
static auto bp = BT_LE_ADV_CONN_NAME; // No TxPower support
// static auto bp = BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
// 					    BT_LE_ADV_OPT_USE_NAME, \
// 					    BT_GAP_ADV_FAST_INT_MIN_2, \
// 					    BT_GAP_ADV_FAST_INT_MAX_2, \
//               BT_LE_ADV_OPT_USE_TX_POWER, \
//               NULL); // txpower - REQUIRES EXT ADV OPT on Zephyr (experimental)
static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  // BT_DATA_BYTES(BT_DATA_TX_POWER, 0x00 ), // See https://github.com/vmware/herald-for-cpp/issues/26
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
// WRONG: Android and iOS need the service UUID Herald is discovered via GATT, not via Advert :-
//        See https://github.com/vmware/herald-for-cpp/issues/26
	// BT_DATA_BYTES(BT_DATA_UUID128_ALL,
	// 	      0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 
	// 				0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
	// ),
//  BT_DATA_BYTES(BT_DATA_TX_POWER, 0x00 ), // 'too big advertising data' if both included

// #ifdef CONFIG_BT_CTLR_TX_PWR
  // BT_DATA_BYTES(BT_DATA_TX_POWER, CONFIG_BT_CTLR_TX_PWR ),
// #endif


ConcreteBLETransmitter::Impl::Impl(
  std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : m_context(std::static_pointer_cast<ZephyrContext>(ctx)), // Herald API guarantees this to be safe
    m_stateManager(bluetoothStateManager),
    m_pds(payloadDataSupplier),
    m_db(bleDatabase),
    delegates(),
    isAdvertising(false)

    HLOGGERINIT(ctx,"Sensor","BLE.ConcreteBLETransmitter")
{
  latestPds = m_pds.get();
}

ConcreteBLETransmitter::Impl::~Impl()
{
  latestPds = NULL;
}

void
ConcreteBLETransmitter::Impl::startAdvertising()
{
  // HTDBG("startAdvertising called");
  if (!BLESensorConfiguration::advertisingEnabled) {
    HTDBG("Sensor Configuration has advertising disabled. Returning.");
    return;
  }
  if (isAdvertising) {
    // HTDBG("Already advertising. Returning.");
    return;
  }

  // Note: TxPower currently disabled due to restricted advert space and the need to include Herald's service. 
  //       See https://github.com/vmware/herald-for-cpp/issues/26
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
  int success = bt_le_adv_start(herald::ble::bp, herald::ble::ad, ARRAY_SIZE(herald::ble::ad), NULL, 0);
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

void
ConcreteBLETransmitter::Impl::stopAdvertising()
{
  // HTDBG("stopAdvertising called");
  if (!BLESensorConfiguration::advertisingEnabled) {
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








ConcreteBLETransmitter::ConcreteBLETransmitter(
  std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : mImpl(std::make_shared<Impl>(ctx,bluetoothStateManager,
      payloadDataSupplier,bleDatabase))
{
  ;
}

ConcreteBLETransmitter::~ConcreteBLETransmitter()
{
  // stop(); // stops using m_addr
}

std::optional<std::shared_ptr<CoordinationProvider>>
ConcreteBLETransmitter::coordinationProvider()
{
  return {};
}

void
ConcreteBLETransmitter::add(const std::shared_ptr<SensorDelegate>& delegate)
{
  mImpl->delegates.push_back(delegate);
}

// 6. Implement any additional transmitter functionality, as required

void
ConcreteBLETransmitter::start()
{
  HDBG("ConcreteBLETransmitter::start");
  if (!BLESensorConfiguration::advertisingEnabled) {
    HDBG("Sensor Configuration has advertising disabled. Returning.");
    return;
  }
  mImpl->m_context->getAdvertiser().registerStopCallback([this] () -> void {
    mImpl->stopAdvertising();
  });
  mImpl->m_context->getAdvertiser().registerStartCallback([this] () -> void {
    mImpl->startAdvertising();
  });
  HDBG("Advertising callbacks registered");

  // Ensure our zephyr context has bluetooth ready
  mImpl->m_context->startBluetooth();

  HDBG("Bluetooth started. Requesting start of adverts");

  mImpl->startAdvertising();
}

void
ConcreteBLETransmitter::stop()
{
  HDBG("ConcreteBLETransmitter::stop");
  if (!BLESensorConfiguration::advertisingEnabled) {
    HDBG("Sensor Configuration has advertising disabled. Returning.");
    return;
  }
  mImpl->stopAdvertising();
}

}
}
