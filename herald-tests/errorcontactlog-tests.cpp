//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"

#include <string>
// #include <iostream>

#include "herald/herald.h"

class DummyLogger : public herald::data::SensorLoggingSink {
public:
  DummyLogger(std::string sub,std::string cat) : subsystem(sub), category(cat), value() {}
  ~DummyLogger() = default;

  void log(herald::data::SensorLoggerLevel level, std::string message) override {
    // std::cout << "DummyLogger::log" << std::endl;
    value = subsystem + "," + category + "," + message;
  }

  std::string subsystem;
  std::string category;
  std::string value;
};

class DummyContext : public herald::Context {
public:
  DummyContext() = default;
  ~DummyContext() = default;

  std::shared_ptr<herald::ble::BluetoothStateManager> getBluetoothStateManager() override { return nullptr;}

  std::shared_ptr<herald::data::SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, 
    const std::string& categoryFor) override
  {
    // std::cout << "DummyContext::getLoggingSink" << std::endl;
    lastLogger = std::make_shared<DummyLogger>(subsystemFor,categoryFor);
    return lastLogger;
  }

  std::shared_ptr<DummyLogger> lastLogger;
};

TEST_CASE("errorcontactlogger-output-dbg", "[errorcontactlogger][output]") {
  SECTION("errorcontactlogger-output-dbg") {
    std::shared_ptr<DummyContext> ctx = std::make_shared<DummyContext>();

    // Create contact logger
    std::shared_ptr<herald::data::ConcretePayloadDataFormatter> pdf = 
      std::make_shared<herald::data::ConcretePayloadDataFormatter>();
    std::shared_ptr<herald::data::ErrorStreamContactLogger> contacts = 
      std::make_shared<herald::data::ErrorStreamContactLogger>(ctx, pdf);

    // test each method to check for output
    // didDetect
    contacts->sensor(
      herald::datatype::SensorType::BLE,
      herald::datatype::TargetIdentifier(
        herald::datatype::Data(std::byte(0x09),6)
      )
    );
    // TODO figure out why this test doesn't work on Windoze
    //  - Seems to be compiling out the HLOGGER, HLOGGERINIT, HERR calls
    //  - Was due to SHARED LIBRARIES on Windows - need to add HERALD_LOG_LEVEL to
    //    TOP LEVEL CMakeLists.txt, not just herald-tests project.

    REQUIRE(ctx->lastLogger->value.size() > 0);
  }
}