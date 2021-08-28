//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>

#include "test-templates.h"

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("ble-database-empty", "[ble][database][ctor][empty]") {
  SECTION("ble-database-empty") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::ble::ConcreteBLEDatabase<CT> db(ctx);

    REQUIRE(db.size() == 0);
  }
}

TEST_CASE("ble-database-callback-verify", "[ble][database][callback][verify]") {
  SECTION("ble-callback-verify") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::ble::ConcreteBLEDatabase<CT> db(ctx);
    DummyBLEDBDelegate delegate;
    db.add(delegate);

    REQUIRE(db.size() == 0);
    REQUIRE(delegate.createCallbackCalled == false);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);

    herald::datatype::Data devMac(std::byte(0x02),6);
    herald::datatype::TargetIdentifier dev(devMac);

    // add in new device
    herald::ble::BLEDevice& devPtr = db.device(dev);
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // add in a second device via the payload, not target identifier
    herald::datatype::PayloadData payload(std::byte(0x1f),6);
    herald::ble::BLEDevice& devPtr2 = db.device(payload);
    REQUIRE(db.size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true); // true because payload set on create
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr2);

    // update a device attribute
    devPtr.rssi(herald::datatype::RSSI{14});
    REQUIRE(db.size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // delete the device
    db.remove(dev);
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // delete non existant

    herald::datatype::Data devMac3(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier dev3(devMac3);

    // add in new device
    db.remove(dev3);
    // nothing should have changed
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

  }
}



TEST_CASE("ble-database-macrotate-samepayload", "[ble][database][macrotate][samepayload]") {
  SECTION("ble-callback-macrotate-samepayload") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::ble::ConcreteBLEDatabase<CT> db(ctx);
    DummyBLEDBDelegate delegate;
    db.add(delegate);

    REQUIRE(db.size() == 0);
    REQUIRE(delegate.createCallbackCalled == false);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);

    herald::datatype::Data devMac(std::byte(0x02),6);
    herald::datatype::TargetIdentifier dev(devMac);

    // add in new device
    herald::ble::BLEDevice& devPtr = db.device(dev);
    auto devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      std::cout << "Mac: " << d.get().identifier() 
                << ", State: " << ((d.get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.get().payloadData().size() > 0 ? d.get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // read its payload
    herald::data::PayloadData devPayload(std::byte(0x55),24);
    devPtr.payloadData(devPayload);
    devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      std::cout << "Mac: " << d.get().identifier() 
                << ", State: " << ((d.get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.get().payloadData().size() > 0 ? d.get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // add a second device via mac
    herald::datatype::Data dev2Mac(std::byte(0x03),6);
    herald::datatype::TargetIdentifier dev2(dev2Mac);
    herald::ble::BLEDevice& dev2Ptr = db.device(dev2);
    devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      std::cout << "Mac: " << d.get().identifier() 
                << ", State: " << ((d.get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.get().payloadData().size() > 0 ? d.get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true); // from first device's payload
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == dev2Ptr);

    // now assume the first device's mac rotates
    herald::datatype::Data devRefreshMac(std::byte(0x08),6);
    herald::datatype::TargetIdentifier devRefresh(devRefreshMac);
    herald::ble::BLEDevice& devRefreshPtr = db.device(devRefresh);
    devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      std::cout << "Mac: " << d.get().identifier() 
                << ", State: " << ((d.get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.get().payloadData().size() > 0 ? d.get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 3);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devRefreshPtr);

    // now fetch its payload - should reduce the total number of devices
    herald::data::PayloadData devRefreshPayload(std::byte(0x55),24); // same as before
    devRefreshPtr.payloadData(devRefreshPayload);
    devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      std::cout << "Mac: " << d.get().identifier() 
                << ", State: " << ((d.get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.get().payloadData().size() > 0 ? d.get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 2); // original targetID with this payload should have been deleted
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devRefreshPtr);
  }
}