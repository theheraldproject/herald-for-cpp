//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "test-templates.h"

#include "herald/herald.h"

#include <cstring>

TEST_CASE("nordicuart-callbacks-basics", "[nordicuart][callbacks][basics]") {
  SECTION("nordicuart-callbacks-basics") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    char buffer[128] = {'\0'};

    auto nusd = herald::ble::nordic_uart::NordicUartSensorDelegate(ctx,[&buffer](void* v,const char* data,std::size_t len) {
      strncpy_s(buffer,data,len);
    });
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    // Call didDetect
    nusd.sensor(herald::datatype::SensorType::BLE, t1);
    std::string expected = "SensorDelegate,didDetect,616161616161\n";
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == std::strcmp(buffer,expected.c_str()));

    // Call didMeasure
    herald::datatype::Proximity prox{.unit=herald::datatype::ProximityMeasurementUnit::RSSI, .value=-41};
    nusd.sensor(herald::datatype::SensorType::BLE, prox, t1);
    expected = "SensorDelegate,didMeasure,616161616161,-41\n"; // NOTE Will this work on every platform??? (Float approximation to string)
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == std::strcmp(buffer,expected.c_str()));

    // Call didMeasureWithPayload
    herald::datatype::Data payload{std::byte(0x55),10}; 
    herald::datatype::Proximity prox2{.unit=herald::datatype::ProximityMeasurementUnit::RSSI, .value=-5};
    nusd.sensor(herald::datatype::SensorType::BLE, prox2, t1, payload);
    expected = "SensorDelegate,didMeasureWithPayload,616161616161,-5,55555555555555555555\n"; // NOTE Will this work on every platform??? (Float approximation to string)
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == std::strcmp(buffer,expected.c_str()));
  }
}

TEST_CASE("nordicuart-callbacks-nocallback", "[nordicuart][callbacks][nocallback]") {
  SECTION("nordicuart-callbacks-nocallback") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    auto nusd = herald::ble::nordic_uart::NordicUartSensorDelegate(ctx);
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    // Call didDetect
    REQUIRE_NOTHROW(nusd.sensor(herald::datatype::SensorType::BLE, t1));

  }
}

TEST_CASE("nordicuart-callbacks-bounds", "[nordicuart][callbacks][bounds]") {
  SECTION("nordicuart-callbacks-bounds") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    char buffer[128] = {'\0'};

    auto nusd = herald::ble::nordic_uart::NordicUartSensorDelegate(ctx,[&buffer](void* v,const char* data,std::size_t len) {
      strncpy_s(buffer,data,len);
    });
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    // Call didMeasureWithPayload - empty payload
    herald::datatype::Data payload; 
    herald::datatype::Proximity prox{.unit=herald::datatype::ProximityMeasurementUnit::RSSI, .value=-5};
    nusd.sensor(herald::datatype::SensorType::BLE, prox, t1, payload);
    std::string expected = "SensorDelegate,didMeasureWithPayload,616161616161,-5,\n";
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == std::strcmp(buffer,expected.c_str()));

    // Call didMeasureWithPayload - overlarge payload
    herald::datatype::Data large{std::byte(0x55),128}; 
    herald::datatype::Proximity prox2{.unit=herald::datatype::ProximityMeasurementUnit::RSSI, .value=-5};
    REQUIRE_NOTHROW(nusd.sensor(herald::datatype::SensorType::BLE, prox2, t1, large));
    expected = "SensorDelegate,didMeasureWithPayload,616161616161,-5,5555555555555555555555555555555555555555555555555555555555555555555555555\n"; // NOTE Will this work on every platform??? (Float approximation to string)
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == std::strcmp(buffer,expected.c_str()));
  }
}