//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef RSSI_H
#define RSSI_H

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class RSSI {
public:
  RSSI(int value); // int
  RSSI(const RSSI& other); // copy
  RSSI(RSSI&& other); // move
  ~RSSI();


  std::size_t hashCode() const;

  std::string toString() const;

  int intValue() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

bool operator==(const RSSI& first, const RSSI& other);

} // end namespace
} // end namespace

#endif