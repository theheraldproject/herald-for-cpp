//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include <string>

#include "herald/herald.h"

TEST_CASE("sensorlogger-output-dbg", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-dbg") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    HTDBG("Simple string");
    std::string r("testout,mytest,Simple string");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTDBG("There are {} strings","two");
    r = "testout,mytest,There are two strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTDBG("There are {} strings",2);
    r = "testout,mytest,There are 2 strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    const char* cc = "some const char";
    HTDBG("There are {} const chars",cc);
    r = "testout,mytest,There are some const char const chars";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTDBG("There are two params 1: {} and 2: {} and some more text", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTDBG("There are two params 1: {} and 2: {} and some more text {} <- but this is blank", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text  <- but this is blank";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTDBG("Too few {} parameters",15,45);
    r = "testout,mytest,Too few 15 parameters";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}

TEST_CASE("sensorlogger-output-log", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-log") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    HTLOG("Simple string");
    std::string r("testout,mytest,Simple string");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTLOG("There are {} strings","two");
    r = "testout,mytest,There are two strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTLOG("There are {} strings",2);
    r = "testout,mytest,There are 2 strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTLOG("There are two params 1: {} and 2: {} and some more text", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTLOG("There are two params 1: {} and 2: {} and some more text {} <- but this is blank", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text  <- but this is blank";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTLOG("Too few {} parameters",15,45);
    r = "testout,mytest,Too few 15 parameters";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}

TEST_CASE("sensorlogger-output-fault", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-fault") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    HTERR("Simple string");
    std::string r("testout,mytest,Simple string");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTERR("There are {} strings","two");
    r = "testout,mytest,There are two strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTERR("There are {} strings",2);
    r = "testout,mytest,There are 2 strings";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTERR("There are two params 1: {} and 2: {} and some more text", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTERR("There are two params 1: {} and 2: {} and some more text {} <- but this is blank", 15, 45);
    r = "testout,mytest,There are two params 1: 15 and 2: 45 and some more text  <- but this is blank";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    HTERR("Too few {} parameters",15,45);
    r = "testout,mytest,Too few 15 parameters";
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}


TEST_CASE("sensorlogger-output-intrinsic", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-intrinsic") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    int i = 37;
    HTLOG("Intrinsic {} type",i);
    std::string r("testout,mytest,Intrinsic 37 type");
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::uint8_t ui8 = 39;
    HTLOG("Intrinsic {} type",ui8);
    r = "testout,mytest,Intrinsic 39 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::size_t st = 128;
    HTLOG("Intrinsic {} type",st);
    r = "testout,mytest,Intrinsic 128 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::uint16_t ui16 = 3737;
    HTLOG("Intrinsic {} type",ui16);
    r = "testout,mytest,Intrinsic 3737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::uint32_t ui32 = 373737;
    HTLOG("Intrinsic {} type",ui32);
    r = "testout,mytest,Intrinsic 373737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::uint32_t ui64 = 3737373737;
    HTLOG("Intrinsic {} type",ui64);
    r = "testout,mytest,Intrinsic 3737373737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::int8_t i8 = -39;
    HTLOG("Intrinsic {} type",i8);
    r = "testout,mytest,Intrinsic -39 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::int16_t i16 = -3737;
    HTLOG("Intrinsic {} type",i16);
    r = "testout,mytest,Intrinsic -3737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::int32_t i32 = -373737;
    HTLOG("Intrinsic {} type",i32);
    r = "testout,mytest,Intrinsic -373737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    std::int32_t i64 = -37373737;
    HTLOG("Intrinsic {} type",i64);
    r = "testout,mytest,Intrinsic -37373737 type";
    INFO(dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}


TEST_CASE("sensorlogger-bug-negativesuccess", "[sensorlogger][output][bug]") {
  SECTION("sensorlogger-bug-negativesuccess") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    int success = -22;
    HTDBG(" - Issue connecting: {}",success);
    std::string r("testout,mytest, - Issue connecting: -22");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}


TEST_CASE("sensorlogger-bug-targetidatend", "[sensorlogger][output][bug]") {
  SECTION("sensorlogger-bug-targetidatend") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    herald::datatype::TargetIdentifier t(herald::datatype::Data(std::byte(0x09),3));

    HTLOG("Complex {}",t);
    std::string r("testout,mytest,Complex 090909");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}


TEST_CASE("sensorlogger-output-data", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-data") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    herald::datatype::Data t(std::byte(0x09),3);

    HTLOG("Complex {} type",t);
    std::string r("testout,mytest,Complex 090909 type");
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}


TEST_CASE("sensorlogger-output-targetidentifier", "[sensorlogger][output]") {
  SECTION("sensorlogger-output-targetidentifier") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include
    // using CT = typename herald::Context<herald::DefaultPlatformType,DummyLoggingSink,DummyBluetoothStateManager>;
    herald::data::SensorLogger logger(ctx.getLoggingSink(),"testout","mytest");

    herald::datatype::TargetIdentifier t(herald::datatype::Data(std::byte(0x09),3));

    std::string r("testout,mytest,Complex 090909 type");
    HTLOG("Complex {} type",t);
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);
  }
}