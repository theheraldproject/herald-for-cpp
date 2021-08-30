//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_CONTEXT_H
#define HERALD_CONTEXT_H

#include "ble/ble_sensor_configuration.h" // TODO abstract this away in to platform class
#include "datatype/date.h"

namespace herald {

/// \brief Compile-time Context class, customisable via template traits. Provides generic access to OS system features.
/// 
/// Covers all cross-cutting concerns methods and helpers to prevent tight coupling between components
/// Currently hard-coded to include Bluetooth relevant radio, but this should be abstracted in future to
/// compile out if Bluetooth support is not needed
template <typename PlatformT,
          typename LoggingSinkT,
          typename BluetoothStateManagerT
         >
struct Context {
  using logging_sink_type = LoggingSinkT;

  Context(PlatformT& platform,LoggingSinkT& sink,BluetoothStateManagerT& bsm) noexcept
    : platform(platform), loggingSink(sink), bleStateManager(bsm), config() {}
  Context(const Context& other) noexcept
    : platform(other.platform), loggingSink(other.loggingSink), bleStateManager(other.bleStateManager), config(other.config)
  {}
  // Context(Context&& other)
  //   : loggingSink(std::move(other.loggingSink)), bleStateManager(std::move(other.bleStateManager))
  // {}
  ~Context() = default;

  
  Context<PlatformT,LoggingSinkT,BluetoothStateManagerT>& operator=(Context& other) noexcept {
    platform = other.platform;
    loggingSink = other.loggingSink;
    bleStateManager = other.bleStateManager;
    return *this;
  }
  
  // \brief Returns a reference to this OS'/runtimes implementation of the LoggingSink
  LoggingSinkT& getLoggingSink() {
    return loggingSink;
  }

  // \brief Returns a reference to this OS'/runtimes implemetation of the Bluetooth State Manager
  BluetoothStateManagerT& getBluetoothStateManager() {
    return bleStateManager;
  }

  // \brief Returns platform-specific implementation. E.g. Zephyr specific calls
  PlatformT& getPlatform() {
    return platform;
  }

  const ble::BLESensorConfiguration& getSensorConfiguration() {
    return config;
  }

  void setSensorConfiguration(ble::BLESensorConfiguration newConfig) {
    config = newConfig;
  }

  datatype::Date getNow() noexcept {
    return platform.getNow();
  }

private:
  PlatformT& platform;
  LoggingSinkT& loggingSink;
  BluetoothStateManagerT& bleStateManager;
  ble::BLESensorConfiguration config;
};

// \brief Default empty platform type for platforms that have no custom functionality
struct DefaultPlatformType {
  datatype::Date getNow() noexcept {
    return datatype::Date();
  }
}; 

} // end namespace

#endif