//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/extended/extended_data.h"
#include "herald/datatype/data.h"

#include <vector>
#include <string>

namespace herald {
namespace payload {
namespace extended {

ConcreteExtendedDataV1::ConcreteExtendedDataV1()
  : mHasData(false),
    sections()
{
  ;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(const ConcreteExtendedDataV1& other)
  : mHasData(other.mHasData),
    sections(other.sections)
{
  ;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(ConcreteExtendedDataV1&& other)
  : mHasData(other.mHasData),
    sections(std::move(other.sections))
{
  ;
}

// ConcreteExtendedDataV1&
// ConcreteExtendedDataV1::operator=(const ConcreteExtendedDataV1& other)
// {
//   mHasData = other.mHasData;
//   sections = other.sections;

//   return *this;
// }

// ConcreteExtendedDataV1&
// ConcreteExtendedDataV1::operator=(ConcreteExtendedDataV1&& other)
// {
//   mHasData = other.mHasData;
//   sections = std::move(other.sections);

//   return *this;
// }

ConcreteExtendedDataV1::~ConcreteExtendedDataV1()
{
  ;
}



bool
ConcreteExtendedDataV1::hasData() const
{
  return mHasData;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint8_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value));
  sections.emplace_back(code, 1, std::move(d));
  mHasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint16_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value >> 8));
  d.push_back(std::byte(value & 0xff));
  sections.emplace_back(code, 1, std::move(d));
  mHasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, float value)
{
  std::vector<std::byte> d;
  for (std::size_t i = sizeof(float);i > 0 ;--i) {
    d.push_back(std::byte(((std::size_t)value) >> (8 * (i - 1))));
  }
  sections.emplace_back(code,sizeof(float), std::move(d));
  mHasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const std::string value)
{
  std::vector<std::byte> d;
  for (auto c : value) {
    d.push_back(std::byte(c));
  }
  sections.emplace_back(code, value.size(), std::move(d));
  mHasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const Data& value)
{
  sections.emplace_back(code, value.size(), value);
  mHasData = true;
}

const std::vector<ConcreteExtendedDataSectionV1>&
ConcreteExtendedDataV1::getSections() const
{
  return sections;
}

std::optional<PayloadData>
ConcreteExtendedDataV1::payload()
{
  if (mHasData) {
    PayloadData result;
    for (auto s : sections) {
      result.append(s.code);
      result.append(s.length);
      result.append(s.data);
    }
    return std::optional<PayloadData>(result);
  }
  return std::optional<PayloadData>(); // empty optional
}

}
}
}
