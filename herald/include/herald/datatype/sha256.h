//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SHA256_H
#define HERALD_SHA256_H

#include "data.h"

#include <memory>

namespace herald {
namespace datatype {

class SHA256 {
public:
  SHA256() noexcept;
  ~SHA256() noexcept;

  Data digest(const Data& with) noexcept;

  void reset() noexcept; // Initialise to all zeros

private:
  // No internal state required for Windows or TinyCrypt or mbedtls
};

}
}

#endif
