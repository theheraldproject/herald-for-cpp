//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/data.h"
#include "herald/ble/ble_mac_address.h"

// nRF Connect SDK includes
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

// C++17 includes
#include <memory>
#include <vector>
#include <cstring>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::data;





class ConcreteBLEReceiver::Impl : public herald::zephyrinternal::Callbacks {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, 
    std::shared_ptr<BLEDatabase> bleDatabase);
  ~Impl();

  // Zephyr OS callbacks
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
      struct net_buf_simple *buf) override;

      
  std::shared_ptr<ZephyrContext> m_context;
  std::shared_ptr<BluetoothStateManager> m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  std::shared_ptr<BLEDatabase> m_db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  SensorLogger logger;
};

ConcreteBLEReceiver::Impl::Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
  std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, 
  std::shared_ptr<BLEDatabase> bleDatabase)
  : m_context(std::static_pointer_cast<ZephyrContext>(ctx)), // Herald API guarantees this to be safe
    m_stateManager(bluetoothStateManager),
    m_pds(payloadDataSupplier),
    m_db(bleDatabase),
    delegates(),
    logger(ctx,"Sensor","BLE.ConcreteBLEReceiver")
{
  ;
}

ConcreteBLEReceiver::Impl::~Impl()
{
  ;
}
  
void
ConcreteBLEReceiver::Impl::scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf)
{
  // logger.debug("Callback");
  // Data advert;
  // // Convert advert scan data to Herald format
  // for (std::uint16_t p = 0;p < buf->len;p++) {
  //   advert.append(std::uint8_t(buf->data[p]));
  // }
  
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  std::string addrStr(addr_str);
  
  logger.debug(addrStr);
  //logger.debug("taskConnectScanResults, didDiscover (device=%s)", (char*)addr_str);
  BLEMacAddress bleMacAddress(addr->a.val);
  logger.debug((std::string)bleMacAddress);

  auto handle = m_db->device(TargetIdentifier((Data)bleMacAddress));
  // logger.debug("taskConnectScanResults, didDiscover (deviceMAC={})", (std::string)bleMacAddress);

  // // The below is now generic Herald

  // // Now pass to relevant BLEDatabase API call
  // auto segments = BLEAdvertParser::extractSegments(advert,0);
  // auto heraldDataSegments = BLEAdvertParser::extractHeraldManufacturerData(BLEAdvertParser::extractManufacturerData(segments));
  // auto device = mImpl->m_db->device(bleMacAddress); // For most devices this will suffice
  // if (0 != heraldDataSegments.size()) {
  //   device = mImpl->m_db->device(heraldDataSegments); // For devices with unnatural (very fast) ble mac rotation, we need to use this rotating data area (some Android devices)
  // }
  // // Seen a new advert, so register discovery
  // device.registerDiscovery(); // TODO separate callback if a CONFIRMED herald device?
  // // Got an RSSI value too, to register that
  // device.rssi(RSSI(rssi));
  // The above will also fire out sensor delegate functions as required
}

// FIGURE OUT WHY INCLUDING BLE RECEIVER CPP FILE EVEN IF UNUSED STOPS THE DEVICE FUNCTIONING AND LOADING USB DRIVE

// 2. Implement discovery callbacks and test

// 4. Implement all other connect/disconnect logic and BLE device lifecycle methods

// 5. Implement Apple device pro-active filtering





namespace zephyrinternal {
  
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

  /**
   * Why is this necessary? Traditional pointer-to-function cannot easily
   * and reliably be wrapped with std::function/bind/mem_fn. We also need
   * the Herald API to use subclasses for each platform, necessitating
   * some sort of static bridge. Not pretty, but works and allows us to
   * prevent nullptr problems
   */
  std::optional<std::shared_ptr<herald::zephyrinternal::Callbacks>> 
    concreteReceiverInstance;
  
  
  void scan_init(void)
  {
    // int err;

    // struct bt_scan_init_param scan_init = {
    //   .connect_if_match = 0, // no auto connect (handled by herald protocol coordinator)
    //   .scan_param = NULL,
    //   .conn_param = BT_LE_CONN_PARAM_DEFAULT
    // };

    // bt_scan_init(&scan_init);
    // bt_scan_cb_register(&scan_cb);

    /*
    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, herald_uuid);
    if (err) {
      printk("Scanning filters cannot be set (err %d)\n", err);

      return;
    }

    err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
    if (err) {
      printk("Filters cannot be turned on (err %d)\n", err);
    }
    */
  }

  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf) {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->scan_cb(addr,rssi,adv_type,buf);
    }
  }

  // void scan_filter_match(struct bt_scan_device_info *device_info,
  //             struct bt_scan_filter_match *filter_match,
  //             bool connectable)
  // {
  //   char addr[BT_ADDR_LE_STR_LEN];

  //   bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

  //   printk("Filters matched. Address: %s connectable: %s\n",
  //     addr, connectable ? "yes" : "no");
  // }

  // void scan_connecting_error(struct bt_scan_device_info *device_info)
  // {
  //   printk("Connecting failed\n");
  // }

  // void scan_connecting(struct bt_scan_device_info *device_info,
  //           struct bt_conn *conn)
  // {
  //   //default_conn = bt_conn_ref(conn);
  // }

  // void scan_filter_no_match(struct bt_scan_device_info *device_info,
  //         bool connectable)
  // {
  //   int err;
  //   struct bt_conn *conn;
  //   char addr[BT_ADDR_LE_STR_LEN];

  //   if (device_info->recv_info->adv_type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
  //     bt_addr_le_to_str(device_info->recv_info->addr, addr,
  //           sizeof(addr));
  //     printk("Direct advertising received from %s\n", addr);
  //     bt_scan_stop();

  //     err = bt_conn_le_create(device_info->recv_info->addr,
  //           BT_CONN_LE_CREATE_CONN,
  //           device_info->conn_param, &conn);

  //     if (!err) {
  //       default_conn = bt_conn_ref(conn);
  //       bt_conn_unref(conn);
  //     }
  //   }
  // }

  // BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_no_match,
  //     scan_connecting_error, scan_connecting);

}






ConcreteBLEReceiver::ConcreteBLEReceiver(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
  std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : mImpl(std::make_shared<Impl>(ctx,bluetoothStateManager,payloadDataSupplier,bleDatabase))
{
  ;
}

ConcreteBLEReceiver::~ConcreteBLEReceiver()
{
  ;
}

std::optional<std::shared_ptr<CoordinationProvider>>
ConcreteBLEReceiver::coordinationProvider()
{
  return {}; // we don't provide this, ConcreteBLESensor provides this. We provide HeraldV1ProtocolProvider
}

void
ConcreteBLEReceiver::add(std::shared_ptr<SensorDelegate> delegate)
{
  mImpl->delegates.push_back(delegate);
}

void
ConcreteBLEReceiver::start()
{
  mImpl->logger.debug("ConcreteBLEReceiver::start");
  herald::ble::zephyrinternal::concreteReceiverInstance = mImpl;

  // Ensure our zephyr context has bluetooth ready
  mImpl->m_context->startBluetooth();
  struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE, // scans for EVERYTHING
		.interval   = 0x0010,
		.window     = 0x0010,
	};

  // now start scanning and register callback
  // using namespace std::placeholders;
  // auto fcb = std::function<void(const bt_addr_le_t *addr, std::int8_t rssi, 
  //   std::uint8_t adv_type, struct net_buf_simple *buf)>(
  //     std::bind(&ConcreteBLEReceiver::Impl::scan_cb,mImpl.get(),_1,_2,_3,_4)
  // );
  // bt_le_scan_cb_t* ptrfcb = fcb;
  zephyrinternal::scan_init();
  int err = bt_le_scan_start(&scan_param, &zephyrinternal::scan_cb); // scan_cb linked via BT_SCAN_CB_INIT call
	// if (err) {
	// 	mImpl->logger.info("Starting scanning failed (err %s)", err);
	// 	return;
	// }
  mImpl->logger.debug("ConcreteBLEReceiver::start completed successfully");
}

void
ConcreteBLEReceiver::stop()
{
  mImpl->logger.debug("ConcreteBLEReceiver::stop");
  
  herald::ble::zephyrinternal::concreteReceiverInstance.reset(); // destroys the shared_ptr not necessarily the underlying value

  // TODO now stop scanning
  int err = bt_le_scan_stop();
  if (err) {
		// mImpl->logger.info("Stopping scanning failed (err {:d})", err);
  }

  // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
  
  mImpl->logger.debug("ConcreteBLEReceiver::stop completed successfully");
}


bool
ConcreteBLEReceiver::immediateSend(Data data, const TargetIdentifier& targetIdentifier)
{
  return false; // TODO implement this
}

bool
ConcreteBLEReceiver::immediateSendAll(Data data)
{
  return false; // TODO implement this
}


// Herald V1 Protocol Provider overrides

bool
ConcreteBLEReceiver::openConnection(const TargetIdentifier& toTarget)
{

}

bool
ConcreteBLEReceiver::closeConnection(const TargetIdentifier& toTarget)
{

}

void
ConcreteBLEReceiver::identifyOS(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::readPayload(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::immediateSend(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

void
ConcreteBLEReceiver::immediateSendAll(Activity activity, CompletionCallback callback)
{
  callback(activity,{});
}

}
}
