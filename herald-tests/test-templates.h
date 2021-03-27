//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TESTS_TEMPLATES_H
#define HERALD_TESTS_TEMPLATES_H

#include "herald/herald.h"

struct DummyLoggingSink {
  DummyLoggingSink() : subsystem(), category(), value() {}
  ~DummyLoggingSink() = default;

  void log(const std::string& sub,const std::string& cat,herald::data::SensorLoggerLevel level, std::string message) {
    // std::cout << "DummyLogger::log" << std::endl;
    value = sub + "," + cat + "," + message;
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
  
  void add(std::shared_ptr<herald::ble::BluetoothStateManagerDelegate> delegate) override {
    ; // no op
  }

  herald::ble::BluetoothState state() override {
    return herald::ble::BluetoothState::poweredOn;
  }
};

#endif