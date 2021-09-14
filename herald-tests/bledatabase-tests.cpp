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
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
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
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
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
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
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
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
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
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 2); // original targetID with this payload should have been deleted
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devRefreshPtr);
  }
}



TEST_CASE("ble-database-device-bymac", "[ble][database][device][bymac]") {
  SECTION("ble-database-device-bymac") {
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
    herald::ble::BLEMacAddress devblema(devMac);

    // add in new device
    herald::ble::BLEDevice& devPtr = db.device(devblema);
    auto devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtr);

    // verify that fetching a different object representing the same mac works and doesn't create a new device
    herald::ble::BLEMacAddress devblema2(devMac);
    herald::ble::BLEDevice& devPtr2 = db.device(devblema2);
    REQUIRE(db.size() == 1);



    // Now create via a TargetIdentifier, and try to lookup via a blemacaddress from that identifier
    herald::datatype::Data devMac2(std::byte(0x04),6);
    herald::datatype::TargetIdentifier ti2(devMac2);

    // add in new device
    herald::ble::BLEDevice& devPtrti = db.device(ti2);
    devices = db.matches([](auto& deviceRef) {
      return true;
    });
    std::cout << "Devices:-" << std::endl;
    for (auto& d : devices) {
      if (!d.has_value()) {
        continue;
      }
      std::cout << "Mac: " << d.value().get().identifier() 
                << ", State: " << ((d.value().get().state() == herald::ble::BLEDeviceState::uninitialised) ? "uninitialised" : "initialised")
                << ", Payload: " << (d.value().get().payloadData().size() > 0 ? d.value().get().payloadData().hexEncodedString() : "Empty") << std::endl;
    }
    REQUIRE(db.size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value().get() == devPtrti);

    // Now fake connecting
    // discovery
    devPtrti.rssi(herald::datatype::RSSI(-14));
    // find services
    std::vector<herald::datatype::UUID> serviceList;
    serviceList.push_back(ctx.getSensorConfiguration().serviceUUID);
    devPtrti.services(serviceList);
    // introspect services
    devPtrti.payloadCharacteristic(ctx.getSensorConfiguration().payloadCharacteristicUUID);
    devPtrti.signalCharacteristic(ctx.getSensorConfiguration().iosSignalCharacteristicUUID);
    devPtrti.operatingSystem(herald::ble::BLEDeviceOperatingSystem::ios);
    // connect
    devPtrti.state(herald::ble::BLEDeviceState::connected);

    // verify that fetching a different object representing the same mac works and doesn't create a new device
    herald::ble::BLEMacAddress devblemati(devMac2);
    herald::ble::BLEDevice& devPtrti2 = db.device(devblemati);
    REQUIRE(db.size() == 2);




    // Now set payload, then lookup via TI and payload, same number of devices
    herald::datatype::PayloadData pl(std::byte(0x19),20);
    devPtrti.payloadData(pl);
    REQUIRE(db.size() == 2);

    herald::ble::BLEDevice& sameDev = db.device(pl);
    REQUIRE(db.size() == 2);
    REQUIRE(sameDev.identifier() == ti2);
    REQUIRE(devPtrti2.payloadData() == pl);
  }
}