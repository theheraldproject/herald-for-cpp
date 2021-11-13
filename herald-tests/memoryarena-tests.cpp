//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "test-templates.h"

#include <memory>

#include "catch.hpp"

#include "herald/herald.h"

TEST_CASE("memoryarena-pagesrequired","[memoryarena][pagesrequired]") {
  SECTION("memoryarena-pagesrequired") {
    auto req1 = herald::datatype::pagesRequired(2048,10);
    REQUIRE(205 == req1);
    auto req2 = herald::datatype::pagesRequired(2048,9);
    REQUIRE(228 == req2);
    auto req3 = herald::datatype::pagesRequired(2048,8);
    REQUIRE(256 == req3);
    auto req4 = herald::datatype::pagesRequired(2048,10);
    REQUIRE(205 == req4);
    auto req5 = herald::datatype::pagesRequired(2048,11);
    REQUIRE(187 == req5);
  }
}

TEST_CASE("memoryarena-size","[memoryarena][size]") {
  SECTION("memoryarena-size") {
    herald::datatype::MemoryArena<2048,10> arena;
    REQUIRE(sizeof(arena) == 2048 + ((205 + 9) / 8) + 2 + 2 + 2); // size of array, Size, PageSize
  }
}

TEST_CASE("memoryarena-set","[memoryarena][set]") {
  SECTION("memoryarena-set") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    herald::util::ByteArrayPrinter bap(ctx);

    herald::datatype::MemoryArena<96,10> arena;
    char value = 'G';
    char nonvalue = 'T';
    auto entry = arena.allocate(89);
    for (int i = 0;i < 89;++i) {
      arena.set(entry,i,nonvalue);
    }
    arena.set(entry,0,value);
    arena.set(entry,5,value);
    arena.set(entry,88,value);
    REQUIRE(value == arena.get(entry,0));
    REQUIRE(nonvalue == arena.get(entry,1));
    REQUIRE(nonvalue == arena.get(entry,4));
    REQUIRE(value == arena.get(entry,5));
    REQUIRE(nonvalue == arena.get(entry,6));
    REQUIRE(nonvalue == arena.get(entry,87));
    REQUIRE(value == arena.get(entry,88));

    std::array<unsigned char,16> buffer;
    for (std::size_t offsetIdx = 0; offsetIdx < (96 / 16);++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }
  }
}

TEST_CASE("memoryarena-set-uninitialised","[memoryarena][set][uninitialised") {
  SECTION("memoryarena-set-uninitialised") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    herald::util::ByteArrayPrinter bap(ctx);

    herald::datatype::MemoryArena<96,10> arena;
    herald::datatype::MemoryArenaEntry notInitialised;

    REQUIRE(0 == notInitialised.byteLength);
    REQUIRE(arena.pagesFree() == 10);

    arena.set(notInitialised, 2, 'a');

    REQUIRE(0 == notInitialised.byteLength);
    REQUIRE(arena.pagesFree() == 10);

    REQUIRE('\0' == arena.get(notInitialised,2));
  }
}

TEST_CASE("memoryarena-reserve","[memoryarena][reserve]") {
  SECTION("memoryarena-reserve") {
    herald::datatype::MemoryArena<2048,10> arena;
    REQUIRE(arena.pagesFree() == 205);
    auto entrySame = arena.allocate(10);
    REQUIRE(arena.pagesFree() == 204);
    arena.reserve(entrySame,10);
    REQUIRE(arena.pagesFree() == 204);
    REQUIRE(entrySame.byteLength == 10);

    auto entryExpanded = arena.allocate(10);
    REQUIRE(arena.pagesFree() == 203);
    arena.reserve(entryExpanded,20);
    REQUIRE(arena.pagesFree() == 202);
    REQUIRE(entryExpanded.byteLength == 20);

    auto entryShorter = arena.allocate(20);
    REQUIRE(arena.pagesFree() == 200);
    arena.reserve(entryShorter,10);
    REQUIRE(arena.pagesFree() == 200);
    REQUIRE(entryShorter.byteLength == 20);
  }
}

TEST_CASE("memoryarena-useall","[memoryarena][useall]") {
  SECTION("memoryarena-useall") {
    herald::datatype::MemoryArena<2048,10> arena;
    REQUIRE(arena.pagesFree() == 205);
    auto entry1 = arena.allocate(512);
    REQUIRE(entry1.startPageIndex == 0);
    REQUIRE(entry1.byteLength == 512);
    auto entry2 = arena.allocate(1024);
    REQUIRE(entry2.startPageIndex == 52);
    REQUIRE(entry2.byteLength == 1024);
    auto entry3 = arena.allocate(491);
    REQUIRE(entry3.startPageIndex == 155);
    REQUIRE(entry3.byteLength == 491); // not 512 as we have overhead due to page length of 10 bytes
    REQUIRE(arena.pagesFree() == 0); // used all pages (not bytes) exactly
    REQUIRE_THROWS([&arena](){
      auto entry4 = arena.allocate(1); // should fail to allocate
    }());
    arena.deallocate(entry1);
    REQUIRE(arena.pagesFree() == 52);
    arena.deallocate(entry3);
    REQUIRE(arena.pagesFree() == 102); // 52 + 50
    arena.deallocate(entry2);
    REQUIRE(arena.pagesFree() == 205);
  }
}

TEST_CASE("memoryarena-entry-rawlocation","[memoryarena][entry][rawlocation]") {
  SECTION("memoryarena-entry-rawlocation") {
    DummyLoggingSink dls;
    DummyBluetoothStateManager dbsm;
    herald::DefaultPlatformType dpt;
    herald::Context ctx(dpt,dls,dbsm); // default context include

    herald::util::ByteArrayPrinter bap(ctx);

    herald::datatype::MemoryArenaEntry emptyEntry;
    herald::datatype::MemoryArena<64,8> arena; // 8 byte boundary for address size offset test calculation!
    REQUIRE(0 == arena.rawStartAddress(emptyEntry));
    auto entry1 = arena.allocate(16);
    const unsigned char* entry1Address = arena.rawStartAddress(entry1);
    REQUIRE(0 != entry1Address);
    auto entry2 = arena.allocate(8);
    const unsigned char* entry2Address = arena.rawStartAddress(entry2);
    REQUIRE(0 != entry2Address);
    auto difference = entry2Address - entry1Address;
    REQUIRE(16 == difference);

    std::array<unsigned char,16> buffer;
    for (std::size_t offsetIdx = 0; offsetIdx < (64 / 16);++offsetIdx) {
      arena.rawCopy(buffer,offsetIdx * 16);
      bap.print(buffer, offsetIdx * 16);
    }

    // Now try copy into a buffer bigger than our arena
    std::array<unsigned char,72> largeBuffer;
    largeBuffer[71] = ((unsigned char)8);
    arena.rawCopy(largeBuffer,0);
    REQUIRE(largeBuffer[71] == ((unsigned char)0));
  }
}