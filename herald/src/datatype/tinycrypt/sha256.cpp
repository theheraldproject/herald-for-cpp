//  Copyright 2021 Herald Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/sha256.h"

#include "herald/datatype/data.h"

// Tinycrypt
#include <tinycrypt/sha256.h>
#include <tinycrypt/constants.h>

#include <string.h>

namespace herald {
namespace datatype {

class SHA256::Impl {
public:
  Impl() noexcept;
  Impl(const Data& initialiseWith) noexcept;
  ~Impl() noexcept;

  // internal state that is OS specific here

};

SHA256::Impl::Impl() noexcept
{
  ;
}

SHA256::Impl::Impl(const Data& initialiseWith) noexcept
{
  ;
}

SHA256::Impl::~Impl() noexcept = default;






SHA256::SHA256() noexcept
  : mImpl(std::make_unique<Impl>())
{
  ;
}

SHA256::SHA256(const Data& initialiseWith) noexcept
  : mImpl(std::make_unique<Impl>(initialiseWith))
{
  ;
}

SHA256::~SHA256() noexcept = default;


Data
SHA256::digest(const Data& with) noexcept
{
  uint8_t output[32]; // 256 bits = 32 bytes
  struct tc_sha256_state_struct state;
	(void)tc_sha256_init(&state);

  /* Simulating multiple fragments */
  uint8_t buffer[32]; // temp buffer
  std::uint8_t temp;
  std::size_t idx = 0; // index in original Data with object
  std::size_t read = 0; // number of bytes read into buffer
  while (idx < with.size()) {
    // copy up to next 32 bytes to buffer
    if (idx + 32 <= with.size()) {
      read = 32;
    } else {
      // less than 32 bytes remain
      read = with.size() - idx;
    }
    for (std::size_t ci = 0; ci < read;ci++) {
      with.uint8(idx + ci, temp); // read one byte at a time
      buffer[ci] = temp;
    }
    // update sha
	  tc_sha256_update(&state, buffer, read);
    // increment index
    idx += read;
  }
  
	(void)tc_sha256_final(output, &state);

  return Data((std::uint8_t*)output,32);
}

// Initialise to all zeros
void
SHA256::reset() noexcept {

}

void
SHA256::reset(const Data& initialiseWith) noexcept {

}

}
}