//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

// #include "herald/datatype/stdlib.h" // hashing of std::pair

#include "herald/zephyr_context.h"
#include "herald/data/zephyr/zephyr_logging_sink.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/ble/bluetooth_state_manager_delegate.h"
#include "herald/datatype/bluetooth_state.h"

#include <memory>
#include <iosfwd>
#include <vector>
#include <map>
#include <utility>

#include <settings/settings.h>
#include <bluetooth/bluetooth.h>

namespace herald {

using namespace herald::data;
using namespace herald::datatype;
using namespace herald::ble;


// ADVERTISER SPECIFICATION
namespace zephyrinternal {
  
Advertiser::Advertiser()
  : startCallback(),
    stopCallback()
{
  // LOG_DBG("Advertiser::ctor");
}

Advertiser::~Advertiser()
{
  // LOG_DBG("Advertiser::dtor");
  startCallback.reset();
  stopCallback.reset();
}

void
Advertiser::stopAdvertising() noexcept
{
  // LOG_DBG("stopAdvertising called");
  if (stopCallback.has_value()) {
    // LOG_DBG("stopAdvertising callback exists. Calling.");
    stopCallback.value()();
  }
}

void
Advertiser::startAdvertising() noexcept
{
  // LOG_DBG("startAdvertising called");
  if (startCallback.has_value()) {
    // LOG_DBG("startAdvertising callback exists. Calling.");
    startCallback.value()();
  }
}

void
Advertiser::registerStopCallback(std::function<void()> cb)
{
  // LOG_DBG("registerStopCallback called");
  stopCallback = cb;
}

void
Advertiser::registerStartCallback(std::function<void()> cb)
{
  // LOG_DBG("registerStartCallback called");
  startCallback = cb;
}

// TODO add functions for unregister, and call from transmitter destructor

} // end namespace

// ZEPHYR CONTEXT PUBLIC INTERFACE METHODS


// Zephyr RTOS implementation of Context
ZephyrContextProvider::ZephyrContextProvider()
  : sink(),
    advertiser(),
    stateDelegates(),
    bluetoothEnabled(false)
{
  ;
}

ZephyrContextProvider::~Context() = default;

ZephyrLoggingSink&
ZephyrContextProvider::getLoggingSink()
{
  return sink;
}

BluetoothStateManager&
ZephyrContextProvider::getBluetoothStateManager()
{
  return *this
}

void
ZephyrContextProvider::add(std::shared_ptr<BluetoothStateManagerDelegate> delegate)
{
  stateDelegates.push_back(delegate);
}

BluetoothState
ZephyrContextProvider::state()
{
  // TODO support detection of Bluetooth being unsupported, and power cycling/resetting states
  if (bluetoothEnabled) {
    return BluetoothState::poweredOn;
  } else {
    return BluetoothState::poweredOff;
  }
}

zephyrinternal::Advertiser&
ZephyrContextProvider::getAdvertiser() noexcept
{
  return advertiser;
}

int 
ZephyrContextProvider::enableBluetooth() noexcept
{
  LOG_INF("Context::enableBluetooth");
  int success;

  // TODO determine if default Zephyr mac address rotation uses Nordic CC3xx, if present
  //  - If not, force an address ourselves using this functionality

  // rotate mac address using onboard crypto
  // auto success = bt_rand(&mImpl->m_addr, 1);
  // if (!success) {
  //   // TODO log
  //   return success;
  // }
  // // TODO set rotation timer, if set (not -1)
  // success = bt_set_id_addr(&mImpl->m_addr);
  // if (!success) {
  //   // TODO log
  //   return success;
  // }
  // // success = bt_ctlr_set_public_addr(&mImpl->m_addr); // TODO figure out if both are required by NRF
  // // if (!success) {
  // //   // TODO log
  // //   return success;
  // // }
  // success = bt_set_name("heraldbeacon"); // TODO vary this by application
  // if (!success) {
  //   // TODO log
  //   return success;
  // }
  success = bt_enable(NULL); // NULL means synchronously enabled
  LOG_INF("bt enable returned");

  // This should only called once
  if (IS_ENABLED(CONFIG_SETTINGS)) {
    LOG_INF("Calling settings load");
    settings_load();
    LOG_INF("settings load returned");
  }
  if (0 == success) {
    bluetoothEnabled = true;

    for (auto delegate : stateDelegates) {
      delegate->bluetoothStateManager(BluetoothState::poweredOn);
    }
  } else {
    LOG_INF("Error enabling Zephyr Bluetooth: %d", success);
  }

  return success;
}

int 
ZephyrContextProvider::startBluetooth() noexcept
{
  if (!bluetoothEnabled) {
    return enableBluetooth();
  }
  return 0; // success
}

int 
ZephyrContextProvider::stopBluetooth() noexcept
{
  for (auto delegate : stateDelegates) {
    delegate->bluetoothStateManager(BluetoothState::poweredOff);
  }
  return 0;
}

void
ZephyrContextProvider::periodicActions() noexcept
{
  // TODO periodic bluetooth actions
  // E.g. determine if we should rotate mac address (if not done for us?)
}



} // end namespace
