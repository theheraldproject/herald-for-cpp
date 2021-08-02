//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "catch.hpp"

#include "herald/herald.h"

#include "test-templates.h"

class DummyBLEDeviceDelegate : public herald::ble::BLEDeviceDelegate {
public:
  DummyBLEDeviceDelegate() : callbackCalled(false), dev(), attr() {};
  ~DummyBLEDeviceDelegate() = default;

  // overrides
  void device(const herald::ble::BLEDevice& device, const herald::ble::BLEDeviceAttribute didUpdate) override {
    callbackCalled = true;
    dev.emplace(std::reference_wrapper<const herald::ble::BLEDevice>(device));
    attr = didUpdate;
  }

  bool callbackCalled;
  std::optional<std::reference_wrapper<const herald::ble::BLEDevice>> dev;
  std::optional<herald::ble::BLEDeviceAttribute> attr;
};

TEST_CASE("ble-device-ctor", "[ble][device][ctor]") {
  SECTION("ble-device-ctor") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate);

    REQUIRE(device.identifier() == id);
    REQUIRE(device.description().size() > 0);
    REQUIRE(((std::string)device).size() > 0);

    REQUIRE(device.timeIntervalSinceConnected() == herald::datatype::TimeInterval::never());
    REQUIRE(device.timeIntervalSinceLastPayloadDataUpdate() == herald::datatype::TimeInterval::never());
    REQUIRE(device.timeIntervalSinceLastUpdate() == herald::datatype::TimeInterval::zero());
    REQUIRE(device.timeIntervalSinceLastWritePayload() == herald::datatype::TimeInterval::never());
    REQUIRE(device.timeIntervalSinceLastWritePayloadSharing() == herald::datatype::TimeInterval::never());
    REQUIRE(device.timeIntervalSinceLastWriteRssi() == herald::datatype::TimeInterval::never());
    
    REQUIRE(device.state() != herald::ble::BLEDeviceState::uninitialised);
    REQUIRE(device.operatingSystem() == herald::ble::BLEDeviceOperatingSystem::unknown);
    REQUIRE(!device.payloadData().has_value());
    // REQUIRE(!device.immediateSendData().has_value());
    REQUIRE(!device.rssi().has_value());
  }
}


TEST_CASE("ble-device-update-state", "[ble][device][update][state]") {
  SECTION("ble-device-update-state") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::ble::BLEDeviceState s = herald::ble::BLEDeviceState::disconnected;
    device.state(s);

    REQUIRE(device.state() == s);
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::state);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-os", "[ble][device][update][os]") {
  SECTION("ble-device-update-os") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::ble::BLEDeviceOperatingSystem s = herald::ble::BLEDeviceOperatingSystem::android;
    device.operatingSystem(s);

    // actual value
    REQUIRE(device.operatingSystem() == s);

    // delegates
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::operatingSystem);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-payload", "[ble][device][update][payload]") {
  SECTION("ble-device-update-payload") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::payload::fixed::ConcreteFixedPayloadDataSupplierV1 pds(826,4,123123123);

    BlankDevice bd;
    herald::datatype::PayloadData payload = pds.payload(herald::datatype::PayloadTimestamp(),bd);
    device.payloadData(payload);

    // actual value
    REQUIRE(device.payloadData() == payload);

    // delegates
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::payloadData);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
    
    REQUIRE(device.timeIntervalSinceLastPayloadDataUpdate() >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(device.timeIntervalSinceLastPayloadDataUpdate() < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-immediatesenddata", "[ble][device][update][immediatesenddata]") {
  SECTION("ble-device-update-immediatesenddata") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::Data raw{std::byte(9), 12};
    herald::datatype::ImmediateSendData isd(raw);
    device.immediateSendData(isd);

    // actual value
    REQUIRE(device.immediateSendData().has_value());
    REQUIRE(device.immediateSendData().value() == isd);

    // delegates
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::immediateSendData);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-rssi", "[ble][device][update][rssi]") {
  SECTION("ble-device-update-rssi") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::RSSI rssi(12);
    device.rssi(rssi);

    // actual value
    REQUIRE(device.rssi().has_value());
    REQUIRE(device.rssi().value() == rssi);

    // delegates
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::rssi);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-txpower", "[ble][device][update][txpower]") {
  SECTION("ble-device-update-txpower") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::ble::BLETxPower tx(12);
    device.txPower(tx);

    // actual value
    REQUIRE(device.txPower().has_value());
    REQUIRE(device.txPower().value() == tx);

    // delegates
    REQUIRE(delegate.callbackCalled);
    const herald::ble::BLEDevice& dev = delegate.dev.value().get();
    REQUIRE(dev == device);
    REQUIRE(delegate.attr.value() == herald::ble::BLEDeviceAttribute::txPower);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}

TEST_CASE("ble-device-update-pseudo", "[ble][device][update][pseudo]") {
  SECTION("ble-device-update-pseudo") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::Data pseudod{std::byte(7),6};
    herald::ble::BLEMacAddress pseudo(pseudod);
    device.pseudoDeviceAddress(pseudo);

    // actual value
    REQUIRE(device.pseudoDeviceAddress().has_value());
    REQUIRE(device.pseudoDeviceAddress().value() == pseudo);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu != herald::datatype::TimeInterval::never()); // pseudo address counts as update
  }
}

TEST_CASE("ble-device-update-receiveOnly", "[ble][device][update][receiveOnly]") {
  SECTION("ble-device-update-receiveOnly") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    REQUIRE(device.receiveOnly() == false); // default - should default to try both read and write
    device.receiveOnly(true);

    // actual value
    REQUIRE(device.receiveOnly() == true);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu == herald::datatype::TimeInterval::zero());
  }
}

TEST_CASE("ble-device-update-ignore", "[ble][device][update][ignore]") {
  SECTION("ble-device-update-ignore") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    REQUIRE(device.ignore() == false); // default - should default to try both read and write
    device.ignore(true);

    // actual value
    REQUIRE(device.ignore() == true);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu == herald::datatype::TimeInterval::zero());
  }
}

TEST_CASE("ble-device-update-payloadchar", "[ble][device][update][payloadchar]") {
  SECTION("ble-device-update-payloadchar") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::IntegerDistributedRandomSource irds;
    herald::datatype::RandomnessGenerator<herald::datatype::IntegerDistributedRandomSource> gen(
      std::move(irds)
    );
    herald::datatype::UUID uuid = herald::datatype::UUID::random(gen);

    device.payloadCharacteristic(uuid);

    // actual value
    REQUIRE(device.payloadCharacteristic().has_value() == true);
    REQUIRE(device.payloadCharacteristic().value() == uuid);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu == herald::datatype::TimeInterval::zero());
  }
}

TEST_CASE("ble-device-update-signalchar", "[ble][device][update][signalchar]") {
  SECTION("ble-device-update-signalchar") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::IntegerDistributedRandomSource irds;
    herald::datatype::RandomnessGenerator<herald::datatype::IntegerDistributedRandomSource> gen(
      std::move(irds)
    );
    herald::datatype::UUID uuid = herald::datatype::UUID::random(gen);

    device.signalCharacteristic(uuid);

    // actual value
    REQUIRE(device.signalCharacteristic().has_value() == true);
    REQUIRE(device.signalCharacteristic().value() == uuid);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu == herald::datatype::TimeInterval::zero());
  }
}

TEST_CASE("ble-device-invalidate-chars", "[ble][device][update][invalidatechars]") {
  SECTION("ble-device-invalidate-chars") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::IntegerDistributedRandomSource irds;
    herald::datatype::RandomnessGenerator<herald::datatype::IntegerDistributedRandomSource> gen(
      std::move(irds)
    );
    herald::datatype::UUID uuids = herald::datatype::UUID::random(gen);
    herald::datatype::UUID uuidp = herald::datatype::UUID::random(gen);

    device.signalCharacteristic(uuids);
    device.payloadCharacteristic(uuidp);

    // actual value
    REQUIRE(device.signalCharacteristic().has_value() == true);
    REQUIRE(device.signalCharacteristic().value() == uuids);
    REQUIRE(device.payloadCharacteristic().has_value() == true);
    REQUIRE(device.payloadCharacteristic().value() == uuidp);

    // clear
    device.invalidateCharacteristics();

    // recheck
    REQUIRE(device.signalCharacteristic().has_value() == false);
    REQUIRE(device.payloadCharacteristic().has_value() == false);

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu == herald::datatype::TimeInterval::zero());
  }
}



TEST_CASE("ble-device-register-rssi", "[ble][device][register][rssi]") {
  SECTION("ble-device-register-rssi") {
    herald::datatype::Data d{std::byte(9),6};
    herald::datatype::TargetIdentifier id{d};
    
    herald::datatype::Date now;
    herald::datatype::Date createdAt = now - herald::datatype::TimeInterval::minutes(1); // forces updated times to be greater than zero

    DummyBLEDeviceDelegate delegate;
    herald::ble::BLESensorConfiguration conf;
    herald::ble::BLEDevice device(conf,id,delegate,createdAt);

    herald::datatype::Date operationAt = now - herald::datatype::TimeInterval::seconds(30);

    device.registerWriteRssi(operationAt);

    // actual value
    REQUIRE(device.timeIntervalSinceLastWriteRssi() >= herald::datatype::TimeInterval::seconds(30));
    REQUIRE(device.timeIntervalSinceLastWriteRssi() < herald::datatype::TimeInterval::never());

    // delegates
    REQUIRE(!delegate.callbackCalled);

    herald::datatype::TimeInterval lu = device.timeIntervalSinceLastUpdate();
    REQUIRE(lu >= herald::datatype::TimeInterval::seconds(0));
    REQUIRE(lu < herald::datatype::TimeInterval::never());
  }
}