//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_CONTEXT_H
#define HERALD_CONTEXT_H

#include "ble/ble_sensor_configuration.h" // TODO abstract this away in to platform class

namespace herald {

///
/// \brief High level abstraction to access platform-specific implemented primitives.
/// 
/// Some platforms require global configuration or static configuration that
/// doesn't map well on to C++ idioms. This class provides an extension capability
/// to allow this linking.
///
// class Context {
// public:
//   Context() = default;
//   ~Context() = default;

//   template <typename SinkT>
//   SensorLogger<SinkT,std::string,std::string> getLogger(const std::string& subsystemFor, const std::string& categoryFor);
//   // virtual std::shared_ptr<SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor) = 0;
//   BluetoothStateManager getBluetoothStateManager();
// };

//class Context; // fwd decl only here


///
/// \brief Default context that just sends logging to stdout
///
// class DefaultContext : public Context {
// public:
//   DefaultContext() = default;
//   ~DefaultContext() = default;

//   std::shared_ptr<BluetoothStateManager> getBluetoothStateManager() override;

//   std::shared_ptr<SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor) override;
// };


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

private:
  PlatformT& platform;
  LoggingSinkT& loggingSink;
  BluetoothStateManagerT& bleStateManager;
  ble::BLESensorConfiguration config;
};

// \brief Default empty platform type for platforms that have no custom functionality
struct DefaultPlatformType {}; 

} // end namespace

#endif