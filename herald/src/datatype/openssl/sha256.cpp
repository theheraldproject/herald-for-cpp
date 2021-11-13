//  Copyright 2021 Herald Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/sha256.h"
#include "herald/datatype/data.h"

// OpenSSL v3 (Apache-2.0 licensed)
#include <openssl/evp.h>

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
  unsigned int n = 32; // size asuint
  uint8_t output[32]; // 256 bits = 32 bytes
  for (int i = 0;i < 32;++i) {
    output[i] = 0;
  }

  EVP_MD_CTX *mdctx;
  EVP_MD *sha256;
   
  mdctx = EVP_MD_CTX_new();
  if (mdctx == NULL) {
    return Data((std::uint8_t*)output,32);
  }
  /*
   * Setting the library ctx to NULL here fetches the algorithm from the providers loaded
   * into the default library context
   */
  sha256 = EVP_MD_fetch(NULL, "SHA2-256", NULL);
  if (sha256 == NULL) {     
    return Data((std::uint8_t*)output,32);
  }
  if (EVP_DigestInit_ex(mdctx, sha256, NULL) != 1) {
    return Data((std::uint8_t*)output,32);
  }
  /* Simulating multiple fragments */
  uint8_t buffer[32]; // temp buffer
  std::uint8_t temp = 0;
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
    EVP_DigestUpdate(mdctx, buffer, read);
    // increment index
    idx += read;
  }

  EVP_DigestFinal_ex(mdctx, output, &n);
    
  /* Explicit fetches return a dynamic object that must be freed */
  EVP_MD_free(sha256);

  return Data((std::uint8_t*)output,32);
}

// Initialise to all zeros
void
SHA256::reset() noexcept {
  ;
}

}
}