//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "catch.hpp"

#include "herald/herald.h"

class DummyBLEDeviceDelegate : public herald::ble::BLEDeviceDelegate {
public:
  DummyBLEDeviceDelegate() : callbackCalled(false), dev(), attr() {};
  ~DummyBLEDeviceDelegate() = default;

  // overrides
  void device(std::shared_ptr<herald::ble::BLEDevice> device, const herald::ble::BLEDeviceAttribute didUpdate) override {
    callbackCalled = true;
    dev = device;
    attr = didUpdate;
  }

  bool callbackCalled;
  std::optional<std::shared_ptr<herald::ble::BLEDevice>> dev;
  std::optional<herald::ble::BLEDeviceAttribute> attr;
};

TEST_CASE("ble-device-ctor", "[ble][device][ctor]") {
  SECTION("ble-device-ctor") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    std::shared_ptr<DummyBLEDeviceDelegate> delegate = std::make_shared<DummyBLEDeviceDelegate>();
    herald::ble::BLEDevice device(id,delegate);

    REQUIRE(device.identifier() == id);
    REQUIRE(device.description().size() > 0);
    REQUIRE(((std::string)device).size() > 0);

    REQUIRE(!device.timeIntervalSinceConnected().has_value());
    REQUIRE(!device.timeIntervalSinceLastPayloadDataUpdate().has_value());
    REQUIRE(!device.timeIntervalSinceLastUpdate().has_value());
    REQUIRE(!device.timeIntervalSinceLastWritePayload().has_value());
    REQUIRE(!device.timeIntervalSinceLastWritePayloadSharing().has_value());
    REQUIRE(!device.timeIntervalSinceLastWriteRssi().has_value());
    
    REQUIRE(!device.state().has_value());
    REQUIRE(!device.operatingSystem().has_value());
    REQUIRE(!device.payloadData().has_value());
    REQUIRE(!device.immediateSendData().has_value());
    REQUIRE(!device.rssi().has_value());
  }
}


TEST_CASE("ble-device-update-state", "[ble][device][update][state]") {
  SECTION("ble-device-update-state") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    std::shared_ptr<DummyBLEDeviceDelegate> delegate = std::make_shared<DummyBLEDeviceDelegate>();
    std::shared_ptr<herald::ble::BLEDevice> device =
      std::make_shared<herald::ble::BLEDevice>(id,delegate,createdAt);

    herald::ble::BLEDeviceState s = herald::ble::BLEDeviceState::disconnected;
    device->state(s);

    REQUIRE(device->state() == s);
    REQUIRE(delegate->callbackCalled);
    std::shared_ptr<herald::ble::BLEDevice> dev = delegate->dev.value();
    REQUIRE(dev == device);
    REQUIRE(delegate->attr.value() == herald::ble::BLEDeviceAttribute::state);

    std::optional<herald::datatype::TimeInterval> lu = device->timeIntervalSinceLastUpdate();
    REQUIRE(lu.has_value());
    REQUIRE(lu.value() >= herald::datatype::TimeInterval::seconds(60));
  }
}

TEST_CASE("ble-device-update-os", "[ble][device][update][os]") {
  SECTION("ble-device-update-os") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    std::shared_ptr<DummyBLEDeviceDelegate> delegate = std::make_shared<DummyBLEDeviceDelegate>();
    std::shared_ptr<herald::ble::BLEDevice> device =
      std::make_shared<herald::ble::BLEDevice>(id,delegate,createdAt);

    herald::ble::BLEDeviceOperatingSystem s = herald::ble::BLEDeviceOperatingSystem::android;
    device->operatingSystem(s);

    // actual value
    REQUIRE(device->operatingSystem() == s);

    // delegates
    REQUIRE(delegate->callbackCalled);
    std::shared_ptr<herald::ble::BLEDevice> dev = delegate->dev.value();
    REQUIRE(dev == device);
    REQUIRE(delegate->attr.value() == herald::ble::BLEDeviceAttribute::operatingSystem);

    std::optional<herald::datatype::TimeInterval> lu = device->timeIntervalSinceLastUpdate();
    REQUIRE(lu.has_value());
    REQUIRE(lu.value() >= herald::datatype::TimeInterval::seconds(60));
  }
}