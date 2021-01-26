//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

// #include "herald/datatype/stdlib.h" // hashing of std::pair

#include "herald/zephyr_context.h"
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

// NOTE: Link Herald to the Zephyr logging system
// Set CONFIG_HERALD_LOG_LEVEL=4 for debug in CMake using add_definitions(-DCONFIG_HERALD_LOG_LEVEL=4 )
//   Defaults to 0 (OFF) - see zephyr_context.h
#include <logging/log.h>

namespace herald {

// THE BELOW IS DONE IN EXACTLY ONE HERALD FILE
LOG_MODULE_REGISTER(heraldlogger, CONFIG_HERALD_LOG_LEVEL);

using namespace herald::data;
using namespace herald::datatype;
using namespace herald::ble;

// HIDDEN LOGGING SINK IMPLEMENTATION FOR ZEPHYR

class ZephyrLoggingSink : public SensorLoggingSink {
public:
  ZephyrLoggingSink(const std::string& subsystem, const std::string& category);
  ~ZephyrLoggingSink();

  void log(SensorLoggerLevel level, std::string message) override;

private:
  std::string m_subsystem;
  std::string m_category;
};



ZephyrLoggingSink::ZephyrLoggingSink(const std::string& subsystem, const std::string& category)
  : m_subsystem(subsystem), m_category(category)
{
  ;
}

ZephyrLoggingSink::~ZephyrLoggingSink() {
  ;
}

void
ZephyrLoggingSink::log(SensorLoggerLevel level, std::string message)
{
  // TODO be more specific? Filter here or in Zephyr?
  std::string finalMessage = m_subsystem + "," + m_category + "," + message;
  switch (level) {
    case SensorLoggerLevel::debug:
      LOG_DBG("%s",log_strdup(finalMessage.c_str()));
      break;
    case SensorLoggerLevel::fault:
      LOG_ERR("%s",log_strdup(finalMessage.c_str()));
      break;
    default:
      LOG_INF("%s",log_strdup(finalMessage.c_str()));
      break;
  }
}



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

// HIDDEN CONTEXT IMPLEMENTATION FOR ZEPHYR


class ZephyrContext::Impl {
public:
  Impl();
  ~Impl();

  // Any Zephyr RTOS specific global handles go here
  bool enabled;

  std::vector<std::shared_ptr<BluetoothStateManagerDelegate>> stateDelegates;

  // pair is subsystem and category
  std::map<std::pair<std::string,std::string>,
           std::shared_ptr<ZephyrLoggingSink>> logSinks;

  zephyrinternal::Advertiser advertiser;
};

ZephyrContext::Impl::Impl()
  : enabled(false),
    stateDelegates(),
    logSinks(),
    advertiser()
{
  ;
}

ZephyrContext::Impl::~Impl()
{
  ;
}




// ZEPHYR CONTEXT PUBLIC INTERFACE METHODS


// Zephyr RTOS implementation of Context
ZephyrContext::ZephyrContext()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

ZephyrContext::~ZephyrContext()
{
  ;
}

std::shared_ptr<SensorLoggingSink>
ZephyrContext::getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor)
{
  std::pair<std::string,std::string> id(subsystemFor,categoryFor);
  auto foundSinkIndex = mImpl->logSinks.find(id);
  if (mImpl->logSinks.end() != foundSinkIndex) {
    return foundSinkIndex->second;
  }
  std::shared_ptr<ZephyrLoggingSink> newSink = std::make_shared<ZephyrLoggingSink>(
    subsystemFor, categoryFor
  );
  mImpl->logSinks.emplace(id, newSink);
  return newSink;
}

std::shared_ptr<BluetoothStateManager>
ZephyrContext::getBluetoothStateManager()
{
  return shared_from_this();
}

void
ZephyrContext::add(std::shared_ptr<BluetoothStateManagerDelegate> delegate)
{
  mImpl->stateDelegates.push_back(delegate);
}

BluetoothState
ZephyrContext::state()
{
  // TODO support detection of Bluetooth being unsupported, and power cycling/resetting states
  if (mImpl->enabled) {
    return BluetoothState::poweredOn;
  } else {
    return BluetoothState::poweredOff;
  }
}

zephyrinternal::Advertiser&
ZephyrContext::getAdvertiser() noexcept
{
  return mImpl->advertiser;
}

int 
ZephyrContext::enableBluetooth() noexcept
{
  LOG_INF("ZephyrContext::enableBluetooth");
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
    mImpl->enabled = true;

    for (auto delegate : mImpl->stateDelegates) {
      delegate->bluetoothStateManager(BluetoothState::poweredOn);
    }
  } else {
    LOG_INF("Error enabling Zephyr Bluetooth: %d", success);
  }

  return success;
}

int 
ZephyrContext::startBluetooth() noexcept
{
  if (!mImpl->enabled) {
    return enableBluetooth();
  }
  return 0; // success
}

int 
ZephyrContext::stopBluetooth() noexcept
{
  for (auto delegate : mImpl->stateDelegates) {
    delegate->bluetoothStateManager(BluetoothState::poweredOff);
  }
  return 0;
}

void
ZephyrContext::periodicActions() noexcept
{
  // TODO periodic bluetooth actions
  // E.g. determine if we should rotate mac address (if not done for us?)
}



} // end namespace
