//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef TARGET_IDENTIFIER_H
#define TARGET_IDENTIFIER_H

#include <string>
#include <memory>

namespace herald {
namespace datatype {

class TargetIdentifier {
public:
  TargetIdentifier();
  TargetIdentifier(const TargetIdentifier& from); // copy ctor

// PLATFORM SPECIFIC CTORS BEGIN
  //TargetIdentifier(BluetoothDevice device);
// PLATFORM SPECIFIC CTORS END

  ~TargetIdentifier();

  bool operator==(const TargetIdentifier& other) const noexcept;
  bool operator==(const std::string& other) const noexcept;

  std::size_t hashCode() const;

  std::string toString() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;

};

} // end namespace
} // end namespace

#endif