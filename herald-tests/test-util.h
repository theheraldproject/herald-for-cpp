//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_TESTS_UTIL_H
#define HERALD_TESTS_UTIL_H

#include "herald/herald.h"

#include <string>

namespace testutil {

/// \brief Returns the C++ platform full file name path (absolute) given the specified filename base (without extension)
std::string fullFilename(const std::string& filenameBase);

/// \brief Compares the CSV file contents with the required base name (no .csv) are equal for all Herald platforms
void validateEqual(const std::string& filenameBase);

}

#endif