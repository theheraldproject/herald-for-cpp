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
    std::shared_ptr<herald::ble::ConcreteBLEDatabase<CT>> db =
      std::make_shared<herald::ble::ConcreteBLEDatabase<CT>>(ctx); // enables shared_from_this

    REQUIRE(db->size() == 0);
  }
}

TEST_CASE("ble-database-callback-verify", "[ble][database][callback][verify]") {
  SECTION("ble-callback-verify") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    std::shared_ptr<herald::ble::ConcreteBLEDatabase<CT>> db =
      std::make_shared<herald::ble::ConcreteBLEDatabase<CT>>(ctx); // enables shared_from_this
    DummyBLEDBDelegate delegate;
    db->add(delegate);

    REQUIRE(db->size() == 0);
    REQUIRE(delegate.createCallbackCalled == false);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);

    herald::datatype::Data devMac(std::byte(0x02),6);
    herald::datatype::TargetIdentifier dev(devMac);

    // add in new device
    std::shared_ptr<herald::ble::BLEDevice> devPtr = db->device(dev);
    REQUIRE(db->size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value() == devPtr);

    // add in a second device via the payload, not target identifier
    herald::datatype::PayloadData payload(std::byte(0x1f),6);
    std::shared_ptr<herald::ble::BLEDevice> devPtr2 = db->device(payload);
    REQUIRE(db->size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == false);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value() == devPtr2);

    // update a device attribute
    devPtr->rssi(herald::datatype::RSSI{14});
    REQUIRE(db->size() == 2);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == false);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value() == devPtr);

    // delete the device
    db->remove(dev);
    REQUIRE(db->size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value() == devPtr);

    // delete non existant

    herald::datatype::Data devMac3(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier dev3(devMac3);

    // add in new device
    db->remove(dev3);
    // nothing should have changed
    REQUIRE(db->size() == 1);
    REQUIRE(delegate.createCallbackCalled == true);
    REQUIRE(delegate.updateCallbackCalled == true);
    REQUIRE(delegate.deleteCallbackCalled == true);
    REQUIRE(delegate.dev.has_value());
    REQUIRE(delegate.dev.value() == devPtr);

  }
}