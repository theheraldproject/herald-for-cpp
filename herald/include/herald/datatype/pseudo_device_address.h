//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PSEUDO_DEVICE_ADDRESS_H
#define PSEUDO_DEVICE_ADDRESS_H

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class PseudoDeviceAddress {
public:
  PseudoDeviceAddress();
  PseudoDeviceAddress(const std::byte* data, std::size_t length);
  ~PseudoDeviceAddress();

  bool operator==(const PseudoDeviceAddress& other) const noexcept;

  std::size_t hashCode() const;

  std::string toString() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif