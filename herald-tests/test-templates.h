//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TESTS_TEMPLATES_H
#define HERALD_TESTS_TEMPLATES_H

#include "herald/herald.h"

#include <iostream>

struct TimeSetPlatformType {
  TimeSetPlatformType()
    : seconds(0) {}
  
  herald::datatype::Date getNow() noexcept {
    return herald::datatype::Date(seconds);
  }

  long seconds;
};

struct DummyLoggingSink {
  DummyLoggingSink() : subsystem(), category(), value() {}
  ~DummyLoggingSink() = default;

  void log(const std::string& sub,const std::string& cat,herald::data::SensorLoggerLevel level, std::string message) {
    value = sub + "," + cat + "," + message;
    std::cout << "DummyLoggingSink::log: " << value << std::endl;
    subsystem = sub;
    category = cat;
  }

  std::string subsystem;
  std::string category;
  std::string value;
};

class DummyBluetoothStateManager : public herald::ble::BluetoothStateManager {
public:
  DummyBluetoothStateManager() = default;
  ~DummyBluetoothStateManager() = default;
  
  void add(herald::ble::BluetoothStateManagerDelegate& delegate) override {
    ; // no op
  }

  herald::ble::BluetoothState state() override {
    return herald::ble::BluetoothState::poweredOn;
  }
};


class DummyBLEDBDelegate : public herald::ble::BLEDatabaseDelegate {
public:
  DummyBLEDBDelegate() : updateCallbackCalled(false), createCallbackCalled(false),
    deleteCallbackCalled(false), dev(), attr() {}
  ~DummyBLEDBDelegate() {}

  // overrides
  void bleDatabaseDidCreate(const std::shared_ptr<herald::ble::BLEDevice>& device) override {
    createCallbackCalled = true;
    dev = device;
  }
  
  void bleDatabaseDidUpdate(const std::shared_ptr<herald::ble::BLEDevice>& device, 
    const herald::ble::BLEDeviceAttribute attribute) override {
    updateCallbackCalled = true;
    dev = device;
    attr = attribute;
  }
  
  void bleDatabaseDidDelete(const std::shared_ptr<herald::ble::BLEDevice>& device) override {
    deleteCallbackCalled = true;
    dev = device;
  }
  
  bool updateCallbackCalled;
  bool createCallbackCalled;
  bool deleteCallbackCalled;
  std::optional<std::shared_ptr<herald::ble::BLEDevice>> dev;
  std::optional<herald::ble::BLEDeviceAttribute> attr;
};

#endif