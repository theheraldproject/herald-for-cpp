//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <memory>
#include <vector>
#include <iostream>

#include "catch.hpp"

#include "herald/herald.h"

class NoOpHeraldV1ProtocolProvider : public herald::ble::HeraldProtocolV1Provider {
public:
  NoOpHeraldV1ProtocolProvider(std::shared_ptr<herald::Context> context,std::shared_ptr<herald::ble::BLEDatabase> bledb)
    : ctx(context) 
      HLOGGERINIT(ctx,"TESTS","NoOpProvider")
  {}
  ~NoOpHeraldV1ProtocolProvider() = default;
  
  bool openConnection(const herald::datatype::TargetIdentifier& toTarget) override {return true;}
  bool closeConnection(const herald::datatype::TargetIdentifier& toTarget) override {return true;}

  void identifyOS(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    HTDBG("identifyOS called");
    cb(act,{});
  }

  void readPayload(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    HTDBG("readPayload called");
    cb(act,{});
  }

  void immediateSend(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    HTDBG("immediateSend called");
    cb(act,{});
  }

  void immediateSendAll(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    HTDBG("immediateSendAll called");
    cb(act,{});
  }

  std::shared_ptr<herald::Context> ctx;
  HLOGGER
};

/*
class MockHeraldV1ProtocolProvider : public herald::ble::HeraldProtocolV1Provider {
public:
  MockHeraldV1ProtocolProvider(std::shared_ptr<herald::ble::BLEDatabase> bledb)
    : db(bledb), hasIdentifiedOs(false), lastDeviceOS(), hasReadPayload(false), lastDevicePayload()
  {}
  ~MockHeraldV1ProtocolProvider() = default;
  
  void identifyOS(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    auto device = db->device(std::get<1>(act.prerequisites.front()).value());
    device->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    cb(act,{});
  }

  void readPayload(herald::engine::Activity act, herald::engine::CompletionCallback cb) override {
    auto device = db->device(std::get<1>(act.prerequisites.front()).value());
    device->payloadData(herald::datatype::Data(std::byte(0x02),2));
    cb(act,{});
  }

  std::shared_ptr<herald::ble::BLEDatabase> db;
  bool hasIdentifiedOs;
  std::optional<herald::datatype::TargetIdentifier> lastDeviceOS;
  bool hasReadPayload;
  std::optional<herald::datatype::TargetIdentifier> lastDevicePayload;
};
*/

TEST_CASE("blecoordinator-ctor", "[coordinator][ctor][basic]") {
  SECTION("blecoordinator-ctor") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    std::vector<herald::engine::FeatureTag> provided = coord->connectionsProvided();
    REQUIRE(provided.size() == 1); // Herald BLE protocol
    auto prot = provided.front();
    REQUIRE(prot == herald::engine::Features::HeraldBluetoothProtocolConnection);

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 0);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 0);
  }
}


TEST_CASE("blecoordinator-unseen-device", "[coordinator][unseen-device][basic]") {
  SECTION("blecoordinator-unseen-device") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 1);
    auto firstConn = conns.front();
    REQUIRE(std::get<0>(firstConn) == herald::engine::Features::HeraldBluetoothProtocolConnection);
    REQUIRE(std::get<1>(firstConn) > 0);
    REQUIRE(std::get<2>(firstConn).has_value());
    REQUIRE(std::get<2>(firstConn).value() == device1);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 2); // determine OS and read payload
    auto firstAct = acts.front();
    REQUIRE(firstAct.prerequisites.size() == 1);
  }
}


TEST_CASE("blecoordinator-android-no-id", "[coordinator][android-no-id][basic]") {
  SECTION("blecoordinator-android-no-id") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 1);
    auto firstConn = conns.front();
    REQUIRE(std::get<0>(firstConn) == herald::engine::Features::HeraldBluetoothProtocolConnection);
    REQUIRE(std::get<1>(firstConn) > 0);
    REQUIRE(std::get<2>(firstConn).has_value());
    REQUIRE(std::get<2>(firstConn).value() == device1);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 1); // just read payload (ID)
    auto firstAct = acts.front();
    REQUIRE(firstAct.prerequisites.size() == 1);
  }
}

TEST_CASE("blecoordinator-two-mixed-no-id", "[coordinator][two-mixed-no-id][basic]") {
  SECTION("blecoordinator-two-mixed-no-id") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    herald::datatype::Data devMac2(std::byte(0x1f),6);
    herald::datatype::TargetIdentifier device2(devMac2);
    std::shared_ptr<herald::ble::BLEDevice> devPtr2 = db->device(device2);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr2->operatingSystem(herald::ble::BLEDeviceOperatingSystem::ios);

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 2); // connections to BOTH devices
    auto firstConn = conns.front();
    REQUIRE(std::get<0>(firstConn) == herald::engine::Features::HeraldBluetoothProtocolConnection);
    REQUIRE(std::get<1>(firstConn) > 0);
    REQUIRE(std::get<2>(firstConn).has_value());
    REQUIRE(std::get<2>(firstConn).value() == device1);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 2); // just read payload (ID) for BOTH devices
    auto firstAct = acts.front();
    REQUIRE(firstAct.prerequisites.size() == 1);
  }
}

TEST_CASE("blecoordinator-got-os-and-id", "[coordinator][got-os-and-id][basic]") {
  SECTION("blecoordinator-got-os-and-id") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr1->payloadData(herald::datatype::PayloadData(std::byte(5),32));

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 0);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 0);
  }
}

TEST_CASE("blecoordinator-got-two-at-different-states", "[coordinator][got-two-at-different-states][basic]") {
  SECTION("blecoordinator-got-two-at-different-states") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    herald::datatype::Data devMac2(std::byte(0x1f),6);
    herald::datatype::TargetIdentifier device2(devMac2);
    std::shared_ptr<herald::ble::BLEDevice> devPtr2 = db->device(device2);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr1->payloadData(herald::datatype::PayloadData(std::byte(5),32));
    devPtr2->operatingSystem(herald::ble::BLEDeviceOperatingSystem::ios);

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 1); // just for ONE device
    auto firstConn = conns.front();
    REQUIRE(std::get<0>(firstConn) == herald::engine::Features::HeraldBluetoothProtocolConnection);
    REQUIRE(std::get<1>(firstConn) > 0);
    REQUIRE(std::get<2>(firstConn).has_value());
    REQUIRE(std::get<2>(firstConn).value() == device2); // device 2 only

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 1); // just read payload (ID) for ONE device
    auto firstAct = acts.front();
    REQUIRE(firstAct.prerequisites.size() == 1);
  }
}


TEST_CASE("blecoordinator-got-immediate-send-targeted", "[coordinator][got-immediate-send-targeted][basic]") {
  SECTION("blecoordinator-got-immediate-send-targeted") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr1->payloadData(herald::datatype::PayloadData(std::byte(5),32));
    devPtr1->immediateSendData(herald::datatype::ImmediateSendData(
      herald::datatype::Data(std::byte(0x01),2)));

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 1);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 1);
  }
}

TEST_CASE("blecoordinator-got-three-at-different-states", "[coordinator][got-three-at-different-states][basic]") {
  SECTION("blecoordinator-got-three-at-different-states") {
    std::shared_ptr<herald::DefaultContext> ctx = 
      std::make_shared<herald::DefaultContext>();
    std::shared_ptr<herald::ble::ConcreteBLEDatabase> db = 
      std::make_shared<herald::ble::ConcreteBLEDatabase>();
    std::shared_ptr<NoOpHeraldV1ProtocolProvider> pp = 
      std::make_shared<NoOpHeraldV1ProtocolProvider>(ctx,db);
    std::shared_ptr<herald::ble::HeraldProtocolBLECoordinationProvider> coord =
      std::make_shared<herald::ble::HeraldProtocolBLECoordinationProvider>(ctx,db,pp);

    herald::datatype::Data devMac1(std::byte(0x1d),6);
    herald::datatype::TargetIdentifier device1(devMac1);
    std::shared_ptr<herald::ble::BLEDevice> devPtr1 = db->device(device1);

    herald::datatype::Data devMac2(std::byte(0x1f),6);
    herald::datatype::TargetIdentifier device2(devMac2);
    std::shared_ptr<herald::ble::BLEDevice> devPtr2 = db->device(device2);

    herald::datatype::Data devMac3(std::byte(0x09),6);
    herald::datatype::TargetIdentifier device3(devMac3);
    std::shared_ptr<herald::ble::BLEDevice> devPtr3 = db->device(device3);

    // Specify that some activity has already happened with the device
    devPtr1->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr1->payloadData(herald::datatype::PayloadData(std::byte(5),32));
    devPtr2->operatingSystem(herald::ble::BLEDeviceOperatingSystem::ios);
    devPtr3->operatingSystem(herald::ble::BLEDeviceOperatingSystem::android);
    devPtr3->payloadData(herald::datatype::PayloadData(std::byte(5),32));
    devPtr3->immediateSendData(herald::datatype::ImmediateSendData(
      herald::datatype::Data(std::byte(0x01),2)));

    std::vector<std::tuple<herald::engine::FeatureTag,herald::engine::Priority,
      std::optional<herald::datatype::TargetIdentifier>>> conns = 
      coord->requiredConnections();
    REQUIRE(conns.size() == 2);
    auto firstConn = conns.front();
    REQUIRE(std::get<0>(firstConn) == herald::engine::Features::HeraldBluetoothProtocolConnection);
    REQUIRE(std::get<1>(firstConn) > 0);
    REQUIRE(std::get<2>(firstConn).has_value());
    REQUIRE(std::get<2>(firstConn).value() == device2);
    auto secondConn = conns[1];
    REQUIRE(std::get<1>(secondConn) > 0);
    REQUIRE(std::get<2>(secondConn).has_value());
    REQUIRE(std::get<2>(secondConn).value() == device3);

    std::vector<herald::engine::Activity> acts = coord->requiredActivities();
    REQUIRE(acts.size() == 2); // just read payload (ID) for ONE device, and immediate send for another
    auto firstAct = acts.front();
    REQUIRE(firstAct.prerequisites.size() == 1);
    REQUIRE(std::get<1>(firstAct.prerequisites.front()).has_value());
    REQUIRE(std::get<1>(firstAct.prerequisites.front()).value() == device2);
    auto secondAct = acts[1];
    REQUIRE(secondAct.prerequisites.size() == 1);
    REQUIRE(secondAct.prerequisites.size() == 1);
    REQUIRE(std::get<1>(secondAct.prerequisites.front()).has_value());
    REQUIRE(std::get<1>(secondAct.prerequisites.front()).value() == device3);
  }
}

// TODO non-device specific activity (broadcast via notify e.g. immediate send all)
