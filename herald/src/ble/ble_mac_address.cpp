//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_mac_address.h"

namespace herald {
namespace ble {

class BLEMacAddress::Impl {
public:
  Impl();
  Impl(const std::uint8_t bytesBigEndian[6]);
  Impl(const Data& from);
  Impl(const BLEMacAddress& from);
  Impl(BLEMacAddress&& from) = delete;
  ~Impl();

  Data data;
};

BLEMacAddress::Impl::Impl()
  : data()
{
  const std::uint8_t empty[6] = {0,0,0,0,0,0};
  data.append(empty, 0, 6);
}

BLEMacAddress::Impl::Impl(const std::uint8_t bytesBigEndian[6])
  : data(bytesBigEndian, 6)
{
  ;
}

// TODO validate this data is at least 6 bytes, and use only the first 6
BLEMacAddress::Impl::Impl(const Data& from)
  : data()
{
  if (from.size() > 6) {
    data.append(from.subdata(0,6));
  } else {
    data.append(from); // could be short
    // Check to see if data was too short, and correct
    if (data.size() < 6) {
      Data additional(std::byte(0), 6 - data.size()); // byte(0) === uint8_t(0)
      data.append(additional);
    }
  }
}

BLEMacAddress::Impl::Impl(const BLEMacAddress& from)
  : data(from.mImpl->data)
{
  ;
}

BLEMacAddress::Impl::~Impl()
{
  ;
}




BLEMacAddress::BLEMacAddress()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

BLEMacAddress::BLEMacAddress(const std::uint8_t bytesBigEndian[6])
  : mImpl(std::make_unique<Impl>(bytesBigEndian))
{
  ;
}

BLEMacAddress::BLEMacAddress(const Data& from)
  : mImpl(std::make_unique<Impl>(from))
{
  ;
}

BLEMacAddress::BLEMacAddress(const BLEMacAddress& from)
  : mImpl(std::make_unique<Impl>(from))
{
  ;
}

BLEMacAddress::~BLEMacAddress()
{
  ;
}



BLEMacAddress&
BLEMacAddress::operator=(const BLEMacAddress& other) noexcept
{
  mImpl->data = other.mImpl->data;
  return *this;
}

BLEMacAddress::operator Data() const
{
  return mImpl->data;
}

// TODO Thorough test of this function with formatting and valid content across numeric range
BLEMacAddress::operator std::string() const
{
  auto reversed = mImpl->data.reversed();
  auto hexReversed = reversed.hexEncodedString();
  std::string result;
  for (std::size_t i = 0;i < hexReversed.size();i += 2) {
    result += hexReversed.at(i);
    result += hexReversed.at(i + 1);
    if (i < 10) {
      result += ":";
    }
  }
  return result;
}

bool
BLEMacAddress::operator==(const BLEMacAddress& other) const
{
  return mImpl->data == other.mImpl->data;
}

void
BLEMacAddress::bytesBigEndian(std::uint8_t bytesBigEndian[6]) const
{
  if (mImpl->data.size() < 6) {
    return;
  }
  for (std::size_t p = 0;p < 6;p++) {
    bytesBigEndian[p] = std::uint8_t(mImpl->data.at(p));
  }
}

}
}
