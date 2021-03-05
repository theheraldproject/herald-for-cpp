//  Copyright 2021 Herald Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/sha256.h"

#include "herald/datatype/data.h"

// Windows specific libraries
#include <windows.h>
#include <stdio.h>
#include <bcrypt.h>

namespace herald {
namespace datatype {


#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

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
SHA256::digest(const Data& with) noexcept {
  // from sample for CNG: https://docs.microsoft.com/en-us/windows/win32/seccng/creating-a-hash-with-cng
  BCRYPT_ALG_HANDLE       hAlg            = NULL;
  BCRYPT_HASH_HANDLE      hHash           = NULL;
  NTSTATUS                status          = STATUS_UNSUCCESSFUL;
  DWORD                   cbData          = 0,
                          cbHash          = 0,
                          cbHashObject    = 0;
  PBYTE                   message         = NULL;
  PBYTE                   pbHashObject    = NULL;
  PBYTE                   pbHash          = NULL;

  Data result;

  //open an algorithm handle
  if(!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
                                              &hAlg,
                                              BCRYPT_SHA256_ALGORITHM,
                                              NULL,
                                              0))) // or BCRYPT_HASH_REUSABLE_FLAG
  {
      wprintf(L"**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", status);
      goto Cleanup;
  }

  //calculate the size of the buffer to hold the hash object
  if(!NT_SUCCESS(status = BCryptGetProperty(
                                      hAlg, 
                                      BCRYPT_OBJECT_LENGTH, 
                                      (PBYTE)&cbHashObject, 
                                      sizeof(DWORD), 
                                      &cbData, /* result length */
                                      0)))
  {
      wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
      goto Cleanup;
  }

  //allocate the hash object on the heap
  pbHashObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbHashObject);
  if(NULL == pbHashObject)
  {
      wprintf(L"**** memory allocation failed\n");
      goto Cleanup;
  }

  //calculate the length of the hash
  if(!NT_SUCCESS(status = BCryptGetProperty(
                                      hAlg, 
                                      BCRYPT_HASH_LENGTH, 
                                      (PBYTE)&cbHash, 
                                      sizeof(DWORD), 
                                      &cbData, 
                                      0)))
  {
      wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
      goto Cleanup;
  }

  //allocate the hash buffer on the heap
  pbHash = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbHash);
  if(NULL == pbHash)
  {
      wprintf(L"**** memory allocation failed\n");
      goto Cleanup;
  }

  //create a hash
  if(!NT_SUCCESS(status = BCryptCreateHash(
                                      hAlg, 
                                      &hHash, 
                                      pbHashObject, 
                                      cbHashObject, 
                                      NULL, 
                                      0, 
                                      0)))
  {
      wprintf(L"**** Error 0x%x returned by BCryptCreateHash\n", status);
      goto Cleanup;
  }
  
  // copy over data
  message = (PBYTE)HeapAlloc (GetProcessHeap (), 0, with.size());
  for (std::size_t i = 0;i < with.size();i++) {
    message[i] = BYTE(with.at(i));
  }

  //hash some data
  if(!NT_SUCCESS(status = BCryptHashData(
                                      hHash,
                                      message,
                                      sizeof(message),
                                      0)))
  {
      wprintf(L"**** Error 0x%x returned by BCryptHashData\n", status);
      goto Cleanup;
  }
  
  //close the hash
  if(!NT_SUCCESS(status = BCryptFinishHash(
                                      hHash, 
                                      pbHash, 
                                      cbHash, 
                                      0)))
  {
      wprintf(L"**** Error 0x%x returned by BCryptFinishHash\n", status);
      goto Cleanup;
  }

  //wprintf(L"Success!\n");

  result = Data(pbHash,cbHash);


Cleanup:

  if(hAlg)
  {
      BCryptCloseAlgorithmProvider(hAlg,0);
  }

  if (hHash)    
  {
      BCryptDestroyHash(hHash);
  }

  if(pbHashObject)
  {
      HeapFree(GetProcessHeap(), 0, pbHashObject);
  }

  if(pbHash)
  {
      HeapFree(GetProcessHeap(), 0, pbHash);
  }

  return result;
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