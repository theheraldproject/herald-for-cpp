//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/ble/ble.h"
#include "herald/data/zephyr/zephyr_logging_sink.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/ble/bluetooth_state_manager_delegate.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/datatype/date.h"

#include <settings/settings.h>
#include <bluetooth/bluetooth.h>

namespace herald {

using namespace herald::data;
using namespace herald::datatype;
using namespace herald::ble;


// ADVERTISER SPECIFICATION
namespace zephyrinternal {
  
Advertiser::Advertiser() noexcept
  : stopCallback(),
    startCallback(),
    restartCallback(),
    isDirtyCallback(),
    customServices()
{
  // LOG_DBG("Advertiser::ctor");
}

Advertiser::~Advertiser() noexcept
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
    startCallback.value()(customServices);
  }
}

void
Advertiser::restartAdvertising() noexcept
{
  // LOG_DBG("restartAdvertising called");
  if (restartCallback.has_value()) {
    // LOG_DBG("restartAdvertising callback exists. Calling.");
    restartCallback.value()(customServices);
  }
}

void
Advertiser::markAdvertAsDirty() noexcept
{
  // LOG_DBG("markAdvertAsDirty called");
  if (isDirtyCallback.has_value()) {
    // LOG_DBG("markAdvertAsDirty callback exists. Calling.");
    isDirtyCallback.value()(customServices);
  }
}

void
Advertiser::registerStopCallback(std::function<void()> cb) noexcept
{
  // LOG_DBG("registerStopCallback called");
  stopCallback = cb;
}

void
Advertiser::registerStartCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept
{
  // LOG_DBG("registerStartCallback called");
  startCallback = cb;
}

void
Advertiser::registerRestartCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept
{
  // LOG_DBG("registerRestartCallback called");
  restartCallback = cb;
}

void
Advertiser::registerIsDirtyCallback(std::function<void(BLEServiceList& customServices)> cb) noexcept
{
  // LOG_DBG("registerIsDirtyCallback called");
  isDirtyCallback = cb;
}

void
Advertiser::unregisterAllCallbacks() noexcept
{
  // LOG_DBG("unregisterAllCallbacks called");
  startCallback.reset();
  restartCallback.reset();
  isDirtyCallback.reset();
  stopCallback.reset();
}

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

ZephyrContextProvider::~ZephyrContextProvider() = default;

ZephyrLoggingSink&
ZephyrContextProvider::getLoggingSink()
{
  return sink;
}

BluetoothStateManager&
ZephyrContextProvider::getBluetoothStateManager()
{
  return *this;
}

void
ZephyrContextProvider::add(BluetoothStateManagerDelegate& delegate)
{
  stateDelegates.emplace_back(delegate);
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

herald::zephyrinternal::Advertiser&
ZephyrContextProvider::getAdvertiser() noexcept
{
  return advertiser;
}

int 
ZephyrContextProvider::enableBluetooth() noexcept
{
  // LOG_INF("Context::enableBluetooth");
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
  // LOG_INF("bt enable returned");

  // This should only called once
  if (IS_ENABLED(CONFIG_SETTINGS)) {
    // LOG_INF("Calling settings load");
    settings_load();
    // LOG_INF("settings load returned");
  }
  if (0 == success) {
    bluetoothEnabled = true;

    for (auto& delegate : stateDelegates) {
      delegate.get().bluetoothStateManager(BluetoothState::poweredOn);
    }
  } else {
    // LOG_INF("Error enabling Zephyr Bluetooth: %d", success);
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
  for (auto& delegate : stateDelegates) {
    delegate.get().bluetoothStateManager(BluetoothState::poweredOff);
  }
  return 0;
}

void
ZephyrContextProvider::periodicActions() noexcept
{
  // TODO periodic bluetooth actions
  // E.g. determine if we should rotate mac address (if not done for us?)
}

datatype::Date
ZephyrContextProvider::getNow() noexcept {
  return datatype::Date();
}

// Methods added in v2.1

bool
ZephyrContextProvider::addCustomService(const herald::ble::BluetoothUUID& serviceId)
{
  advertiser.customServices.add(BLEService{.uuid=serviceId,.characteristics={}});
  advertiser.markAdvertAsDirty();
  return true;
}

void
ZephyrContextProvider::removeCustomService(const herald::ble::BluetoothUUID& serviceId)
{
  advertiser.customServices.remove(serviceId);
  advertiser.markAdvertAsDirty();
}

bool
ZephyrContextProvider::addCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, 
  const herald::ble::BluetoothUUID& charId, const herald::ble::BLECharacteristicType& charType, 
  const herald::ble::BLECallbacks& callbacks)
{
  advertiser.customServices.add(BLEService(serviceId,BLECharacteristicList{BLECharacteristic(charId,charType,callbacks)}));
  advertiser.markAdvertAsDirty();
  return true;
}

void
ZephyrContextProvider::removeCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, 
  const herald::ble::BluetoothUUID& charId)
{
  bool changed = false;
  for (auto& svc : advertiser.customServices) {
    if (svc == serviceId) {
      svc.characteristics.remove(charId);
      changed = true;
    }
  }
  if (changed) {
    advertiser.markAdvertAsDirty();
  }
}

void
ZephyrContextProvider::notifyAllSubscribers(const herald::ble::BluetoothUUID& serviceId, 
  const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue)
{
  // TODO add functionality here
  ;
}

void
ZephyrContextProvider::notifySubscriber(const herald::ble::BluetoothUUID& serviceId, 
  const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue, const herald::ble::BLEMacAddress& toNotify)
{
  // TODO add functionality here
  ;
}

} // end namespace
