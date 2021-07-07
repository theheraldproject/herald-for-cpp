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


TEST_CASE("crossplatform-k-matchingkeyseed", "[.][payload][k-matchingkeyseed][basic]") {
  SECTION("crossplatform-k-matchingkeyseed") {
    // Check output file can be created
    auto fn = testutil::fullFilename("kMatchingSeed.csv");
    INFO("Output filename: " << fn);
    std::ofstream cppOut(fn);
    cppOut << "day,matchingSeed" << std::endl;

    herald::payload::simple::SecretKey ks1((std::byte)0, 2048);
    herald::payload::simple::K myK(2048, 2000, 240); // have to generate 2000 to get the right end key value
    auto last(herald::payload::simple::F::h(ks1));
    herald::payload::simple::MatchingKey nks;
    // Note the indexes below are the value of i. I.e. today is day 0, tomorrow is day 1, etc.
    // So we start with day 2001 (at index 2000)
    cppOut << 2000 << "," << herald::datatype::Base64String::encode(last).encoded() << std::endl;
    for (int day = 1999; day >= 0; --day) {
      nks = herald::payload::simple::F::h(herald::payload::simple::F::t(last));
      auto mks = herald::datatype::Base64String::encode(nks).encoded();
      cppOut << day << ","
             << mks << std::endl;
      last = nks;
    }
    cppOut.close(); // flushes and closes

    // Now ensure our output matches the other platforms'
    testutil::validateEqual("kMatchingSeed.csv");
  }
}
