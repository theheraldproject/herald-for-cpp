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
  // Data advert;
  // // Convert advert scan data to Herald format
  // for (std::uint16_t p = 0;p < buf->len;p++) {
  //   advert.append(std::uint8_t(buf->data[p]));
  // }
  
	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
  std::string addrStr(addr_str);


  logger.debug("taskConnectScanResults, didDiscover (device={})", addrStr);
  BLEMacAddress bleMacAddress(addr->a.val);
  logger.debug("taskConnectScanResults, didDiscover (deviceMAC={})", (std::string)bleMacAddress);

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
  /**
   * Why is this necessary? Traditional pointer-to-function cannot easily
   * and reliably be wrapped with std::function/bind/mem_fn. We also need
   * the Herald API to use subclasses for each platform, necessitating
   * some sort of static bridge. Not pretty, but works and allows us to
   * prevent nullptr problems
   */
  std::optional<std::shared_ptr<herald::zephyrinternal::Callbacks>> 
    concreteReceiverInstance;
  
  void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf) {
    if (concreteReceiverInstance.has_value()) {
      concreteReceiverInstance.value()->scan_cb(addr,rssi,adv_type,buf);
    }
  }

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


void
ConcreteBLEReceiver::add(std::shared_ptr<SensorDelegate> delegate)
{
  mImpl->delegates.push_back(delegate);
}

void
ConcreteBLEReceiver::start()
{
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
  int err = bt_le_scan_start(&scan_param, 
    //std::bind(&ConcreteBLEReceiver::Impl::scan_cb,mImpl.get(),_1,_2,_3,_4)

    // [this](const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
    //    struct net_buf_simple *buf) -> void {
    //      mImpl->scan_cb(addr,rssi,adv_type,buf);
    //   }

    // fcb

    &zephyrinternal::scan_cb
  );
	if (err) {
		mImpl->logger.info("Starting scanning failed (err {:d})", err);
		return;
	}
}

void
ConcreteBLEReceiver::stop()
{
  herald::ble::zephyrinternal::concreteReceiverInstance.reset(); // destroys the shared_ptr not necessarily the underlying value

  // TODO now stop scanning
  int err = bt_le_scan_stop();
  if (err) {
		// mImpl->logger.info("Stopping scanning failed (err {:d})", err);
  }

  // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
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

}
}
