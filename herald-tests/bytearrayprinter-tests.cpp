//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include "catch.hpp"

#include <string>

#include "herald/herald.h"

TEST_CASE("bytearrayprinter-basic", "[bytearrayprinter][basic]") {
  SECTION("bytearrayprinter-basic") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm);

    herald::util::ByteArrayPrinter bap(ctx);
    std::array<unsigned char,16> buffer;
    auto& arena = herald::datatype::Data::getArena();
    // RESET AS OTHERWISE ARENA IS AFFECTED BY OTHER TESTS OF THE DATA CLASS!!!
    arena.reset();
    const std::size_t initialPagesFree = arena.pagesFree();

    // Allocate a Data instance
    // Note this allocates a Data object, then copies into the Data object owned by the ti - so we need to check page 2!
    herald::datatype::TargetIdentifier ti(herald::datatype::Data(std::byte(0x01),6));
    for (std::size_t offsetIdx = 0; offsetIdx < 1;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    std::size_t pagesUsed = initialPagesFree - arena.pagesFree();
    REQUIRE(1 == pagesUsed);
    std::string r("util,ByteArrayPrinter,0: 00 00 00 00 00 00 00 00  01 01 01 01 01 01 00 00");
    INFO("Expected: " << r.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);

    // validate next memory location is empty too before we allocate
    for (std::size_t offsetIdx = 1; offsetIdx < 2;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    std::string r2e("util,ByteArrayPrinter,16: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00");
    INFO("Expected: " << r2e.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r2e.c_str(),dls.value.c_str()) == 0);

    // Allocate another Target Identifier
    herald::datatype::TargetIdentifier ti2(herald::datatype::Data(std::byte(0x02),6));
    for (std::size_t offsetIdx = 1; offsetIdx < 2;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    pagesUsed = initialPagesFree - arena.pagesFree();
    REQUIRE(2 == pagesUsed);
    std::string r2("util,ByteArrayPrinter,16: 02 02 02 02 02 02 00 00  00 00 00 00 00 00 00 00");
    INFO("Expected: " << r2.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r2.c_str(),dls.value.c_str()) == 0);

    // And a third

    // validate next memory location is empty too before we allocate
    // No need, it's the second byte on the above, and we've already checked for zeros

    // Allocate another Target Identifier
    herald::datatype::TargetIdentifier ti3(herald::datatype::Data(std::byte(0x03),6));
    for (std::size_t offsetIdx = 1; offsetIdx < 2;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    pagesUsed = initialPagesFree - arena.pagesFree();
    REQUIRE(3 == pagesUsed);
    std::string r3("util,ByteArrayPrinter,16: 02 02 02 02 02 02 00 00  03 03 03 03 03 03 00 00");
    INFO("Expected: " << r3.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r3.c_str(),dls.value.c_str()) == 0);

    // Now deallocate the second
    herald::datatype::MemoryArenaEntry entry{.startPageIndex=2, .byteLength=6}; // Note this is a tad naughty - the ti2 variable above is now potentially overwritten in RAM
    arena.deallocate(entry);

    for (std::size_t offsetIdx = 1; offsetIdx < 2;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    pagesUsed = initialPagesFree - arena.pagesFree();
    REQUIRE(2 == pagesUsed);
    std::string r4("util,ByteArrayPrinter,16: 00 00 00 00 00 00 00 00  03 03 03 03 03 03 00 00");
    INFO("Expected: " << r4.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r4.c_str(),dls.value.c_str()) == 0);
  }
}

TEST_CASE("bytearrayprinter-bytes", "[bytearrayprinter][bytes]") {
  SECTION("bytearrayprinter-bytes") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm);

    herald::util::ByteArrayPrinter bap(ctx);
    std::array<unsigned char,16> buffer;
    auto& arena = herald::datatype::Data::getArena();
    // RESET AS OTHERWISE ARENA IS AFFECTED BY OTHER TESTS OF THE DATA CLASS!!!
    arena.reset();
    const std::size_t initialPagesFree = arena.pagesFree();

    // Allocate a Data instance
    // Note this allocates a Data object, then copies into the Data object owned by the ti - so we need to check page 2!
    std::uint8_t rawData[] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    herald::datatype::Data data(rawData,24);

    std::size_t pagesUsed = initialPagesFree - arena.pagesFree();
    REQUIRE(3 == pagesUsed);

    for (std::size_t offsetIdx = 0; offsetIdx < 1;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    std::string r("util,ByteArrayPrinter,0: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f");
    INFO("Expected: " << r.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r.c_str(),dls.value.c_str()) == 0);


    for (std::size_t offsetIdx = 1; offsetIdx < 2;++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
    // Note: At the 8 byte boundary as the unallocated space could have any random value
    std::string r2("util,ByteArrayPrinter,16: 10 11 12 13 14 15 16 17  00 00 00 00 00 00 00 00");
    INFO("Expected: " << r2.c_str());
    INFO("Received: " << dls.value.c_str());
    REQUIRE(strcmp(r2.c_str(),dls.value.c_str()) == 0);
  }
}