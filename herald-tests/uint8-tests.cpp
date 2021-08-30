//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <fstream>
#include <string>

#include "catch.hpp"

#include "herald/herald.h"

#include "test-util.h"

TEST_CASE("crossplatform-uint8", "[.][uint8][crossplatform]") {
  SECTION("crossplatform-uint8") {
    // Check output file can be created
    auto fn = testutil::fullFilename("uint8.csv");
    INFO("Output filename: " << fn);
    std::ofstream cppOut(fn);
    cppOut << "value,data" << std::endl;
    std::uint8_t result;
    for (std::uint8_t i = std::numeric_limits<uint8_t>::min(); i < std::numeric_limits<uint8_t>::max(); ++i) {
      herald::datatype::Data data;
      data.append(i);
      // REQUIRE(data.uint8(0,result));
      // REQUIRE(i == result);
      cppOut << (unsigned int)i << "," << herald::datatype::Base64String::encode(data).encoded() << std::endl;
    }
    // now generate last value too (doing it in the loop results in an infinite loop in C++!)
    std::uint8_t i = std::numeric_limits<uint8_t>::max();
    herald::datatype::Data data;
    data.append(i);
    // REQUIRE(data.uint8(0,result));
    // REQUIRE(i == result);
    cppOut << (unsigned int)i << "," << herald::datatype::Base64String::encode(data).encoded() << std::endl;

    cppOut.close();

    // Now ensure our output matches the other platforms'
    testutil::validateEqual("uint8.csv");
  }
}
