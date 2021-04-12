//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>
#include <optional>
#include <iostream>

#include "test-templates.h"

#include "catch.hpp"

#include "herald/herald.h"

/**
 * This set of tests uses the BLECoordinator and a Mock Herald V1 Protocol provider
 * to test the iteration functionality of the core Coordinator class
 */

template <typename CoordProvT>
class MockSensor : public herald::ble::Sensor {
public:
  MockSensor(CoordProvT& provider) : cp(provider) {}
  ~MockSensor() = default;

  
  void add(const std::shared_ptr<herald::ble::SensorDelegate>& delegate) override {}
  void start() override {}
  void stop() override {}

  /** For complex sensor coordination support, if required - Since v1.2-beta3 **/
  std::optional<std::reference_wrapper<herald::engine::CoordinationProvider>> coordinationProvider() override {
    return std::optional<std::reference_wrapper<herald::engine::CoordinationProvider>>(cp);
  }

  CoordProvT& cp;
};

template <typename ContextT, typename BLEDBT>
class NoOpHeraldV1ProtocolProvider : public herald::ble::HeraldProtocolV1Provider {
public:
  NoOpHeraldV1ProtocolProvider(ContextT& context,BLEDBT& bledb)
    : ctx(context) 
      HLOGGERINIT(ctx,"TESTS","NoOpProvider")
  {}
  ~NoOpHeraldV1ProtocolProvider() = default;
  
  // FOR PLATFORMS WITH STD::ASYNC:-
  // void openConnection(const herald::datatype::TargetIdentifier& toTarget,
  //   const herald::ble::HeraldConnectionCallback& connCallback) override {
  //   connCallback(toTarget, true);
  // }

  // void closeConnection(const herald::datatype::TargetIdentifier& toTarget,
  //   const herald::ble::HeraldConnectionCallback& connCallback) override {
  //   connCallback(toTarget,false);
  // }

  // void serviceDiscovery(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("serviceDiscovery called");
  //   cb(act,{});
  // }

  // void readPayload(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("readPayload called");
  //   cb(act,{});
  // }

  // void immediateSend(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("immediateSend called");
  //   cb(act,{});
  // }

  // void immediateSendAll(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("immediateSendAll called");
  //   cb(act,{});
  // }

  // FOR PLATFORMS WITHOUT STD::SYNC
  bool openConnection(const herald::datatype::TargetIdentifier& toTarget) override {
    return true;
  }
  bool closeConnection(const herald::datatype::TargetIdentifier& toTarget) override {
    return false;
  }
  
  void restartScanningAndAdvertising() override {}

  std::optional<herald::engine::Activity> serviceDiscovery(herald::engine::Activity act) override {
    HTDBG("serviceDiscovery called");
    return {};
  }

  std::optional<herald::engine::Activity> readPayload(herald::engine::Activity act) override {
    HTDBG("readPayload called");
    return {};
  }

  std::optional<herald::engine::Activity> immediateSend(herald::engine::Activity act) override {
    HTDBG("immediateSend called");
    return {};
  }

  std::optional<herald::engine::Activity> immediateSendAll(herald::engine::Activity act) override {
    HTDBG("immediateSendAll called");
    return {};
  }

  ContextT& ctx;
  HLOGGER(ContextT);
};

template <typename ContextT, typename BLEDBT>
class MockHeraldV1ProtocolProvider : public herald::ble::HeraldProtocolV1Provider {
public:
  MockHeraldV1ProtocolProvider(ContextT& context,BLEDBT& bledb)
    : ctx(context), db(bledb), hasIdentifiedOs(false), lastDeviceOS(), hasReadPayload(false), lastDevicePayload(),
      hasImmediateSend(false), lastImmediateSend(), hasImmediateSendAll(false), lastImmediateSendAll()
      HLOGGERINIT(ctx,"TESTS","MockHeraldV1ProtocolProvider")
  {}
  ~MockHeraldV1ProtocolProvider() = default;
  
  // FOR PLATFORMS WITH STD::ASYNC:-
  // void openConnection(const herald::datatype::TargetIdentifier& toTarget,
  //   const herald::ble::HeraldConnectionCallback& connCallback) override {
  //   connCallback(toTarget, true);
  // }
  // void closeConnection(const herald::datatype::TargetIdentifier& toTarget,
  //   const herald::ble::HeraldConnectionCallback& connCallback) override {
  //   connCallback(toTarget,false);
  // }

  // void serviceDiscovery(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("serviceDiscovery called");
  //   auto device = db->device(std::get<1>(act.prerequisites.front()).value());
  //   device->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
  //   hasIdentifiedOs = true;
  //   lastDeviceOS = device->identifier();
  //   cb(act,{});
  // }

  // void readPayload(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("readPayload called");
  //   auto device = db->device(std::get<1>(act.prerequisites.front()).value());
  //   device->payloadData(herald::datatype::Data(std::byte(0x02),2));
  //   hasReadPayload = true;
  //   lastDevicePayload = device->identifier();
  //   cb(act,{});
  // }

  // void immediateSend(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("immediateSend called");
  //   auto device = db->device(std::get<1>(act.prerequisites.front()).value());
  //   hasImmediateSend = true;
  //   lastImmediateSend = device->identifier();
  //   device->clearImmediateSendData();
  //   cb(act,{});
  // }

  // void immediateSendAll(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
  //   HTDBG("immediateSendAll called");
  //   auto device = db->device(std::get<1>(act.prerequisites.front()).value());
  //   hasImmediateSendAll = true;
  //   lastImmediateSendAll = device->identifier();
  //   device->clearImmediateSendData();
  //   cb(act,{});
  // }

  // FOR PLATFORMS WITHOUT:-
  bool openConnection(const herald::datatype::TargetIdentifier& toTarget) override {
    return true;
  }
  bool closeConnection(const herald::datatype::TargetIdentifier& toTarget) override {
    return false;
  }
  
  void restartScanningAndAdvertising() override {}

  std::optional<herald::engine::Activity> serviceDiscovery(herald::engine::Activity act) override {
    HTDBG("serviceDiscovery called");
    auto device = db.device(std::get<1>(act.prerequisites.front()).value());
    std::vector<herald::datatype::UUID> heraldServiceList;
    herald::ble::BLESensorConfiguration cfg;
    heraldServiceList.push_back(cfg.serviceUUID);
    device->services(heraldServiceList);
    device->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    hasIdentifiedOs = true;
    lastDeviceOS = device->identifier();
    return {};
  }

  std::optional<herald::engine::Activity> readPayload(herald::engine::Activity act) override {
    HTDBG("readPayload called");
    auto device = db.device(std::get<1>(act.prerequisites.front()).value());
    device->payloadData(herald::datatype::Data(std::byte(0x02),2));
    hasReadPayload = true;
    lastDevicePayload = device->identifier();
    return {};
  }

  std::optional<herald::engine::Activity> immediateSend(herald::engine::Activity act) override {
    HTDBG("immediateSend called");
    auto device = db.device(std::get<1>(act.prerequisites.front()).value());
    hasImmediateSend = true;
    lastImmediateSend = device->identifier();
    device->clearImmediateSendData();
    return {};
  }

  std::optional<herald::engine::Activity> immediateSendAll(herald::engine::Activity act) override {
    HTDBG("immediateSendAll called");
    auto device = db.device(std::get<1>(act.prerequisites.front()).value());
    hasImmediateSendAll = true;
    lastImmediateSendAll = device->identifier();
    device->clearImmediateSendData();
    return {};
  }

  ContextT& ctx;
  BLEDBT& db;
  
  bool hasIdentifiedOs;
  std::optional<herald::datatype::TargetIdentifier> lastDeviceOS;
  
  bool hasReadPayload;
  std::optional<herald::datatype::TargetIdentifier> lastDevicePayload;

  bool hasImmediateSend;
  std::optional<herald::datatype::TargetIdentifier> lastImmediateSend;

  bool hasImmediateSendAll;
  std::optional<herald::datatype::TargetIdentifier> lastImmediateSendAll;

  HLOGGER(ContextT);
};




TEST_CASE("coordinator-complex-iterations", "[coordinator][iterations][complex]") {
  // create our BLE coordinator
  DummyLoggingSink dls;
  DummyBluetoothStateManager dbsm;
  herald::DefaultPlatformType dpt;
  herald::Context ctx(dpt,dls,dbsm); // default context include
  using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
  herald::ble::ConcreteBLEDatabase db(ctx);
  MockHeraldV1ProtocolProvider pp(ctx,db);
  herald::ble::HeraldProtocolBLECoordinationProvider coord(ctx,db,pp);
  using CPT = herald::ble::HeraldProtocolBLECoordinationProvider<
    CT,
    herald::ble::ConcreteBLEDatabase<CT>,
    MockHeraldV1ProtocolProvider<CT,herald::ble::ConcreteBLEDatabase<CT>>
  >;

  // Mock Sensor
  MockSensor<CPT> mockSensor(coord);

  // register ble coordinator
  herald::engine::Coordinator<CT> c(ctx);
  c.add(mockSensor); // registers the BLE coordinator
  c.start();

  // section wide data definitions
  herald::datatype::Data devMac1(std::byte(0x1d),6);
  herald::datatype::TargetIdentifier device1(devMac1);

  herald::datatype::Data devMac2(std::byte(0x1f),6);
  herald::datatype::TargetIdentifier device2(devMac2);
  // std::shared_ptr<herald::ble::BLEDevice> devPtr2 = db->device(device2);

  herald::datatype::Data devMac3(std::byte(0x09),6);
  herald::datatype::TargetIdentifier device3(devMac3);
  // std::shared_ptr<herald::ble::BLEDevice> devPtr3 = db->device(device3);
  
  SECTION("blecoordinator-complex-iterations-01-device1") {
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db.device(device1);

    // std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
    //   std::optional<herald::datatype::TargetIdentifier>>> conns = 
    //   coord->requiredConnections();

    // Now perform one iteration
    c.iteration();
    
    // check provider used
    REQUIRE(pp.hasIdentifiedOs == true);
    REQUIRE(pp.lastDeviceOS == device1);
    REQUIRE(pp.hasReadPayload == false);
    REQUIRE(pp.hasImmediateSend == false);
    REQUIRE(pp.hasImmediateSendAll == false);
  //}

  // second iteration should read payload
    c.iteration();
    REQUIRE(pp.hasIdentifiedOs == true);
    REQUIRE(pp.lastDeviceOS == device1);
    REQUIRE(pp.hasReadPayload == true);
    REQUIRE(pp.lastDevicePayload == device1);
    REQUIRE(pp.hasImmediateSend == false);
    REQUIRE(pp.hasImmediateSendAll == false);
  
  //SECTION("blecoordinator-complex-iterations-02-immediatesend-device1") {
    //std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    // std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
    //   std::optional<herald::datatype::TargetIdentifier>>> conns = 
    //   coord->requiredConnections();

    // TODO MARK DEVICE FOR IMMEDIATE SEND IN PROVIDER
    devPtr1->immediateSendData(herald::datatype::Data(std::byte(0x09),4));

    // Now perform one iteration
    c.iteration();
    
    // check provider used
    REQUIRE(pp.hasIdentifiedOs == true);
    REQUIRE(pp.lastDeviceOS == device1);
    REQUIRE(pp.hasReadPayload == true);
    REQUIRE(pp.lastDevicePayload == device1);
    REQUIRE(pp.hasImmediateSend == true);
    REQUIRE(pp.lastImmediateSend == device1);
    REQUIRE(pp.hasImmediateSendAll == false);
  //}
  
  //SECTION("blecoordinator-complex-iterations-03-noactivity") {
    // No changes to required activity

    // Now perform one iteration
    c.iteration();
    
    // check provider used
    REQUIRE(pp.hasIdentifiedOs == true);
    REQUIRE(pp.lastDeviceOS == device1);
    REQUIRE(pp.hasReadPayload == true);
    REQUIRE(pp.lastDevicePayload == device1);
    REQUIRE(pp.hasImmediateSend == true);
    REQUIRE(pp.lastImmediateSend == device1);
    REQUIRE(pp.hasImmediateSendAll == false);
  }

  c.stop();
}
