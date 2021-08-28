//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include <string>

#include "herald/herald.h"

TEST_CASE("errorcontactlogger-output-dbg", "[errorcontactlogger][output]") {
  SECTION("errorcontactlogger-output-dbg") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    
    // Create contact logger
    herald::data::ConcretePayloadDataFormatter pdf;
    herald::data::ErrorStreamContactLogger contacts(ctx, pdf);

    // test each method to check for output
    // didDetect
    contacts.sensor(
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