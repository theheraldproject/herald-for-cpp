//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TESTS_TEMPLATES_H
#define HERALD_TESTS_TEMPLATES_H

#include "herald/herald.h"

#include <iostream>

class BlankDevice : public herald::Device {
public:
  BlankDevice() : id(herald::datatype::Data(std::byte(10),6)) {};
  ~BlankDevice() = default;

  herald::datatype::Date created() const {
    return herald::datatype::Date(14);
  }
  herald::datatype::TimeInterval timeIntervalSinceLastUpdate() const {
    return herald::datatype::TimeInterval(12);
  }
  const herald::datatype::TargetIdentifier& identifier() const {
    return id;
  }
  void identifier(const herald::datatype::TargetIdentifier& toCopyFrom) {}

private:
  herald::ble::TargetIdentifier id;
};

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

  bool addCustomService(const herald::ble::BluetoothUUID& serviceId) override {
    return true;
  }

  void removeCustomService(const herald::ble::BluetoothUUID& serviceId) override {
    ;
  }

  bool addCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::ble::BLECharacteristicType& charType, const herald::ble::BLECallbacks& callbacks) override {
    return true;
  }

  void removeCustomServiceCharacteristic(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId) override {
    ;
  }

  void notifyAllSubscribers(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue) override {
    ;
  }

  void notifySubscriber(const herald::ble::BluetoothUUID& serviceId, const herald::ble::BluetoothUUID& charId, const herald::datatype::Data& newValue, const herald::ble::BLEMacAddress& toNotify) override {
    ;
  }
};


class DummyBLEDBDelegate : public herald::ble::BLEDatabaseDelegate {
public:
  DummyBLEDBDelegate() : updateCallbackCalled(false), createCallbackCalled(false),
    deleteCallbackCalled(false), dev(), attr() {}
  ~DummyBLEDBDelegate() {}

  // overrides
  void bleDatabaseDidCreate(const herald::ble::BLEDevice& device) override {
    createCallbackCalled = true;
    dev.emplace(device);
  }
  
  void bleDatabaseDidUpdate(const herald::ble::BLEDevice& device, 
    const herald::ble::BLEDeviceAttribute attribute) override {
    updateCallbackCalled = true;
    dev.emplace(device);
    attr.emplace(attribute);
  }
  
  void bleDatabaseDidDelete(const herald::ble::BLEDevice& device) override {
    deleteCallbackCalled = true;
    dev.emplace(device);
  }
  
  bool updateCallbackCalled;
  bool createCallbackCalled;
  bool deleteCallbackCalled;
  std::optional<std::reference_wrapper<const herald::ble::BLEDevice>> dev;
  std::optional<herald::ble::BLEDeviceAttribute> attr;
};

template <std::size_t Sz>
struct DummyRSSISource {
  using value_type = herald::analysis::Sample<herald::datatype::RSSI>; // allows AnalysisRunner to introspect this class at compile time

  DummyRSSISource(const std::size_t srcDeviceKey, 
    herald::analysis::SampleList<herald::analysis::Sample<herald::datatype::RSSI>,Sz>&& data)
    : key(srcDeviceKey), data(std::move(data)), lastAddedAt(0), lastRunAdded(0), hasRan(false) {};
  ~DummyRSSISource() = default;

  template <typename RunnerT>
  void run(std::uint64_t timeTo, RunnerT& runner) {
    // push through data at default rate
    lastRunAdded = 0;
    for (auto& v: data) {
      // devList.push(v.taken,v.value); // copy data over (It's unusual taking a SampleList and sending to a SampleList)
      auto sampleTime = v.taken.secondsSinceUnixEpoch();
      // Only push data that hasn't been pushed yet, otherwise we get an ever increasing sample list
      if ((!hasRan || sampleTime > lastAddedAt) && (sampleTime <= timeTo)) {
        ++lastRunAdded;
        runner.template newSample<RSSI>(key,v);
      }
    }
    runner.run(Date(timeTo));
    lastAddedAt = timeTo;
    hasRan = true;
  }

  std::uint64_t getLastRunAdded() {
    return lastRunAdded;
  }

private:
  std::size_t key;
  herald::analysis::SampleList<herald::analysis::Sample<herald::datatype::RSSI>,Sz> data;
  std::uint64_t lastAddedAt;
  std::uint64_t lastRunAdded;
  bool hasRan;
};

#endif