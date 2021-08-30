//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include "test-templates.h"

#include "herald/herald.h"

#include <string.h>

TEST_CASE("nordicuart-callbacks-basics", "[nordicuart][callbacks][basics]") {
  SECTION("nordicuart-callbacks-basics") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    char buffer[128] = {'\0'};

    auto nusd = herald::ble::nordic_uart::NordicUartSensorDelegate(ctx,[&buffer](void* v,const char* data,std::size_t len) {
      strncpy_s(buffer,128,data,len);
    });
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    // Call didDetect
    nusd.sensor(herald::datatype::SensorType::BLE, t1);
    std::string expected = "SensorDelegate,didDetect,616161616161\n";
    INFO("Callback value: '" << buffer << "', expected: '" << expected << "'");
    REQUIRE(0 == strcmp(buffer,expected.c_str()));
  }
}

TEST_CASE("nordicuart-callbacks-nocallback", "[nordicuart][callbacks][nocallback]") {
  SECTION("nordicuart-callbacks-nocallback") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    char buffer[128] = {'\0'};

    auto nusd = herald::ble::nordic_uart::NordicUartSensorDelegate(ctx);
    herald::datatype::Data d{std::byte('a'),6};
    herald::datatype::TargetIdentifier t1(d);

    // Call didDetect
    REQUIRE_NOTHROW(nusd.sensor(herald::datatype::SensorType::BLE, t1));

  }
}