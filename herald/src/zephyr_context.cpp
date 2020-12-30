//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"

#include <memory>
#include <iostream>

#include <bluetooth/bluetooth.h>

// NOTE: Link Herald to the Zephyr logging system
// Set CONFIG_HERALD_LOG_LEVEL=4 for debug in CMake using add_definitions(-DCONFIG_HERALD_LOG_LEVEL=4 )
//   Defaults to 0 (OFF) - see zephyr_context.h
#include <logging/log.h>
LOG_MODULE_REGISTER(herald, CONFIG_HERALD_LOG_LEVEL);

namespace herald {

class ZephyrContext::Impl {
public:
  Impl();
  ~Impl();

  // Any Zephyr RTOS specific global handles go here
  bt_addr_le_t m_addr;
  bool enabled;
};

ZephyrContext::Impl::Impl()
  : m_addr({ 0, { { 0, 0, 0, 0, 0, 0 } } }) // TODO make this random, and rotate too (if not beacon)
    ,
    enabled(false)
{
  ;
}

ZephyrContext::Impl::~Impl()
{
  ;
}


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

std::ostream& 
ZephyrContext::getLoggingSink(const std::string& requestedFor)
{
  // TODO return a better stream for this platform (E.g. Serial output)
  return std::cout;
}

std::shared_ptr<BluetoothStateManager>
ZephyrContext::getBluetoothStateManager()
{
  return std::shared_ptr<BluetoothStateManager>(this);
}

BluetoothState
ZephyrContext::state()
{
  // TODO support detection of Bluetooth unsupported, and power cycling/resetting states
  if (mImpl->enabled) {
    return BluetoothState::poweredOn;
  } else {
    return BluetoothState::poweredOff;
  }
}

int 
ZephyrContext::enableBluetooth() noexcept
{
  int success;
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
  return 0;
}

void
ZephyrContext::periodicActions() noexcept
{
  // TODO periodic bluetooth actions
  // E.g. determine if we should rotate mac address (if not done for us?)
  // Check if ble receiver needs to go talk to anyone (new device, payload, rssi reading)
}

} // end namespace
