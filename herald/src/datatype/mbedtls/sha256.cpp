//  Copyright 2021 Herald Contributors
//  SPDX-License-Identifier: Apache-2.0
//  Includes elements from https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-tls-hashing/file/c68a6dc8d494/main.cpp/
//  which are Copyright (C) 2016, ARM Limited, All Rights Reserved
//  and also distributed under the Apache-2.0 license
//

#include "herald/datatype/sha256.h"

#include "herald/datatype/data.h"

// mbedtld specific libraries
#include "mbedtls/sha256.h" /* SHA-256 only */
#include "mbedtls/md.h"     /* generic interface */ 
#if DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif
#include "mbedtls/platform.h" 
#include <string.h>

namespace herald {
namespace datatype {

SHA256::SHA256() noexcept
{
  ;
}

SHA256::~SHA256() noexcept = default;


Data
SHA256::digest(const Data& with) noexcept
{
  unsigned char output[32]; // 256 bits = 32 bytes

  mbedtls_sha256_context ctx2;
 
  mbedtls_sha256_init(&ctx2);
  mbedtls_sha256_starts(&ctx2, 0); /* SHA-256, not 224 */

  /* Simulating multiple fragments */
  unsigned char buffer[32]; // temp buffer
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
    for (std::size_t ci = 0; ci < read;++ci) {
      with.uint8(idx + ci, temp); // read one byte at a time
      buffer[ci] = (unsigned char)temp;
    }
    // update sha
    mbedtls_sha256_update(&ctx2, buffer, read);
    // increment index
    idx += read;
  }
  
  mbedtls_sha256_finish(&ctx2, output);
  mbedtls_sha256_free(&ctx2);

  return Data((std::uint8_t*)output,32);
}

// Initialise to all zeros
void
SHA256::reset() noexcept {

}


}
}