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
  RSSI(int value);
  RSSI(RSSI&& other);
  ~RSSI();

  bool operator==(const RSSI& other);

  std::size_t hashCode() const;

  std::string toString() const;

  int intValue() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif