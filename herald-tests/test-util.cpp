//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++17 FS library

#include "catch.hpp"

#include "test-util.h"

namespace testutil {

namespace fs = std::filesystem;
using path = std::filesystem::path;

std::string
fullFilename(const std::string& filename)
{
  path basedir = std::filesystem::current_path().parent_path().parent_path() / "herald-tests";
  path cppCsv = basedir / "cpp" / filename;
  // Cannot check existence as this function is generally used to generate the output file name
  return cppCsv.string();
}

void
validateEqual(const std::string& filename)
{
  path basedir = std::filesystem::current_path().parent_path().parent_path() / "herald-tests"; // throws filesystem_error if path does not exist
  
  // get reference to each file
  path cppCsv = basedir / "cpp" / filename;
  path androidCsv = basedir / "android" / filename;
  path iosCsv = basedir / "ios" / filename;
  // ensure files all exist
  REQUIRE(fs::exists(cppCsv));
  REQUIRE(fs::exists(androidCsv));
  REQUIRE(fs::exists(iosCsv));

  // read line by line
  std::string cppLine;
  std::string androidLine;
  std::string iosLine;
  std::ifstream cppIn(cppCsv.string());
  std::ifstream androidIn(androidCsv.string());
  std::ifstream iosIn(iosCsv.string());
  // ensure files are all readable
  REQUIRE(cppIn.is_open());
  REQUIRE(androidIn.is_open());
  REQUIRE(iosIn.is_open());

  int line = 1;

  // always read one line per file, for EVERY file per iteration (& not &&)
  while (std::getline(cppIn, cppLine)) {
    if (!std::getline(androidIn, androidLine)) {
      break;
    }
    if (!std::getline(iosIn, iosLine)) {
      break;
    }
    // compare lines
    INFO("C++ file line is " << line);
    REQUIRE(androidLine == iosLine); // idiot check first (error lies elsewhere)
    REQUIRE(cppLine == androidLine);
    REQUIRE(cppLine == iosLine);
    // above errors if not equal
    ++line;
  }
  // ensure files are all at an end (We've not got less data than the other files)
  // - Get current file position
  auto cppCurrentPos = cppIn.tellg();
  auto androidCurrentPos = androidIn.tellg();
  auto iosCurrentPos = iosIn.tellg();
  // - Seek to end
  cppIn.seekg(0, std::ios::end);
  androidIn.seekg(0, std::ios::end);
  iosIn.seekg(0, std::ios::end);
  // - ensure current position is also the same as the end (we haven't moved)
  REQUIRE(cppCurrentPos == cppIn.tellg());
  REQUIRE(androidCurrentPos == androidIn.tellg());
  REQUIRE(iosCurrentPos == iosIn.tellg());
}

}