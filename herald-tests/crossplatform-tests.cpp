//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <fstream>
#include <string>

#include "catch.hpp"

#include "herald/herald.h"

#include "test-util.h"

TEST_CASE("crossplatform-contact-identifier", "[.][payload][crossplatform][basic]") {
  SECTION("crossplatform-contact-identifier") {
    // Check output file can be created
    auto fn = testutil::fullFilename("contactIdentifier.csv");
    INFO("Output filename: " << fn);
    std::ofstream cppOut(fn);
    cppOut << "day,period,matchingKey,contactKey,contactIdentifier" << std::endl;

    herald::payload::simple::SecretKey ks1((std::byte)0, 2048);
    herald::payload::simple::K myK(2048, 2000, 240); // have to generate 2000 to get the right end key value
    for (int day = 0; day <= 10; ++day) {
      auto mk = herald::datatype::Base64String::encode(myK.matchingKey(ks1, day)).encoded();
      for (int period = 0; period <= 240; ++period) {
        cppOut << day << "," << period << ","
               << mk << ","
               << herald::datatype::Base64String::encode(myK.contactKey(ks1, day, period)).encoded() << ","
               << herald::datatype::Base64String::encode(myK.contactIdentifier(ks1, day, period)).encoded() << std::endl; // TODO verify EOL character is consistent
      }
    }
    cppOut.close(); // flushes and closes

    // Now ensure our output matches the other platforms'
    testutil::validateEqual("contactIdentifier.csv");
  }
}
