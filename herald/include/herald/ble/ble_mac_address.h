//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_MAC_ADDRESS_H
#define BLE_MAC_ADDRESS_H

#include "../datatype/data.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

/**
 * Enables strong typing of a Mac Address
 */
class BLEMacAddress {
public:
  BLEMacAddress();
  BLEMacAddress(const std::uint8_t bytesBigEndian[6]);
  BLEMacAddress(const Data& from);
  BLEMacAddress(const BLEMacAddress& from);
  BLEMacAddress(BLEMacAddress&& from) = delete;
  // TODO add constructor that uses a const randomness source reference
  ~BLEMacAddress();

  operator Data() const;
  /** Print mac address format with colon separators, little endian **/
  operator std::string() const;
  bool operator==(const BLEMacAddress& other) const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};


}
}

#endif
