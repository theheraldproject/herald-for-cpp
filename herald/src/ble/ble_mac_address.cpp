//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_mac_address.h"

namespace herald {
namespace ble {

BLEMacAddress::BLEMacAddress()
  : data()
{
  const std::uint8_t empty[6] = {0,0,0,0,0,0};
  data.append(empty, 0, 6);
}

BLEMacAddress::BLEMacAddress(const std::uint8_t bytesBigEndian[6])
  : data(bytesBigEndian, 6)
{
  ;
}

BLEMacAddress::BLEMacAddress(const Data& from)
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

BLEMacAddress::BLEMacAddress(const BLEMacAddress& from)
  : data(from.data)
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
  data = other.data;
  return *this;
}

BLEMacAddress::operator Data() const
{
  return data;
}

// TODO Thorough test of this function with formatting and valid content across numeric range
BLEMacAddress::operator std::string() const
{
  auto reversed = data.reversed();
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
  return data == other.data;
}

bool
BLEMacAddress::operator!=(const BLEMacAddress& other) const
{
  return data != other.data;
}

void
BLEMacAddress::bytesBigEndian(std::uint8_t bytesBigEndian[6]) const
{
  if (data.size() < 6) {
    return;
  }
  for (std::size_t p = 0;p < 6;p++) {
    bytesBigEndian[p] = std::uint8_t(data.at(p));
  }
}

}
}
