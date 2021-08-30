//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/simple/f.h"
#include "herald/datatype/data.h"
#include "herald/datatype/sha256.h"

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;

namespace F {

[[maybe_unused]]
Data h(const Data& data) noexcept {
  SHA256 sha;
  return sha.digest(data);
}

[[maybe_unused]]
Data t(const Data& data) noexcept {
  return t(data,data.size() / 2);
}

[[maybe_unused]]
Data t(const Data& data, int n) noexcept {
  return data.subdata(0,n);
}

[[maybe_unused]]
Data xorData(const Data& left, const Data& right) noexcept {
  Data result;
  // note: we're ensuring we don't have an out of bound index (has effect of XOR with 0 as we set value above)
  for (std::size_t i = 0;i < right.size() && i < left.size();i++) {
    result.append(std::byte(((int)left.at(i)) ^ ((int)right.at(i))));
  }
  if (right.size() < left.size()) {
    for (std::size_t i = left.size();i < left.size();i++) {
      result.append(left.at(i));
    }
  }
  return result;
}

} // end namespace f

}
}
}
