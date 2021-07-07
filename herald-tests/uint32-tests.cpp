//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <fstream>
#include <string>

#include "catch.hpp"

#include "herald/herald.h"

#include "test-util.h"

TEST_CASE("crossplatform-uint32", "[.][uint32][crossplatform]") {
  SECTION("crossplatform-uint32") {
    // Check output file can be created
    auto fn = testutil::fullFilename("uint32.csv");
    INFO("Output filename: " << fn);
    std::ofstream cppOut(fn);
    cppOut << "value,data" << std::endl;
    std::uint32_t result;
    std::uint32_t i(1);
    std::uint32_t max(std::numeric_limits<uint32_t>::max() / 7);
    while (i <= max) {
      herald::datatype::Data data;
      data.append(i);
      cppOut << (unsigned long)i << "," << herald::datatype::Base64String::encode(data).encoded() << std::endl;
      i *= 7;
    }

    cppOut.close();

    // Now ensure our output matches the other platforms'
    testutil::validateEqual("uint32.csv");
  }
}
