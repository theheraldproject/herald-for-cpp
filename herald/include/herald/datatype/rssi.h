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

  RSSI& operator=(const RSSI& other); // copy assign
  RSSI& operator=(RSSI&& other); // move assign
  
  bool operator==(const RSSI& other) const noexcept;
  bool operator!=(const RSSI& other) const noexcept;
  bool operator<(const RSSI& other) const noexcept;
  bool operator<=(const RSSI& other) const noexcept;
  bool operator>(const RSSI& other) const noexcept;
  bool operator>=(const RSSI& other) const noexcept;

  std::size_t hashCode() const noexcept;

  operator std::string() const noexcept;

  int intValue() const noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif