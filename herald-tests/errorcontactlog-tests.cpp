//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include <string>
// #include <iostream>

#include "herald/herald.h"

// class LoggingSink {
// public:
//   LoggingSink() : subsystem(), category(), value() {}
//   ~LoggingSink() = default;

//   void log(const std::string& sub,const std::string& cat,herald::data::SensorLoggerLevel level, std::string message) {
//     // std::cout << "DummyLogger::log" << std::endl;
//     value = sub + "," + cat + "," + message;
//     subsystem = sub;
//     category = cat;
//   }

//   std::string subsystem;
//   std::string category;
//   std::string value;
// };

// class DummyContext : public herald::BluetoothStateManager {
// public:
//   DummyContext() : sink() {};
//   ~DummyContext() = default;

//   herald::ble::BluetoothStateManager& getBluetoothStateManager() {
//     return *this;
//   }

//   LoggingSink& getLoggingSink()
//   {
//     return sink;
//   }

//   void add(std::shared_ptr<herald::BluetoothStateManagerDelegate> delegate) override {
//     // ignore
//   }

//   herald::datatype::BluetoothState state() override {
//     return herald::datatype::BluetoothState::poweredOn;
//   }

//   LoggingSink sink;
// };

TEST_CASE("errorcontactlogger-output-dbg", "[errorcontactlogger][output]") {
  SECTION("errorcontactlogger-output-dbg") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    
    // Create contact logger
    std::shared_ptr<herald::data::ConcretePayloadDataFormatter> pdf = 
      std::make_shared<herald::data::ConcretePayloadDataFormatter>();
    std::shared_ptr<herald::data::ErrorStreamContactLogger<CT>> contacts = 
      std::make_shared<herald::data::ErrorStreamContactLogger<CT>>(ctx, pdf);

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

    REQUIRE(dls.value.size() > 0);
  }
}