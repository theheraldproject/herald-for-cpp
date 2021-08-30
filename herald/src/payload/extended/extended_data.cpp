//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/payload/extended/extended_data.h"
#include "herald/datatype/data.h"

#include <string>

namespace herald {
namespace payload {
namespace extended {

ConcreteExtendedDataV1::ConcreteExtendedDataV1()
  : sections(),
    inUse(0)
{
  ;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(const ConcreteExtendedDataV1& other)
  : sections(other.sections),
    inUse(other.inUse)
{
  ;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(ConcreteExtendedDataV1&& other)
  : sections(std::move(other.sections)),
    inUse(other.inUse)
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
  return inUse > 0;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint8_t value)
{
  if (inUse >= sections.size()) {
    return;
  }
  sections[inUse].code = code;
  sections[inUse].length = sizeof(float);
  sections[inUse].data.append(std::byte(value));
  ++inUse;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint16_t value)
{
  if (inUse >= sections.size()) {
    return;
  }
  sections[inUse].code = code;
  sections[inUse].length = sizeof(float);
  sections[inUse].data.append(std::byte(value >> 8));
  sections[inUse].data.append(std::byte(value & 0xff));
  ++inUse;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, float value)
{
  if (inUse >= sections.size()) {
    return;
  }
  sections[inUse].code = code;
  sections[inUse].length = sizeof(float);
  for (std::size_t i = sizeof(float);i > 0 ;--i) {
    sections[inUse].data.append(std::byte(((std::size_t)value) >> (8 * (i - 1))));
  }
  ++inUse;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const std::string value)
{
  if (inUse >= sections.size()) {
    return;
  }
  sections[inUse].data.append(value);
  sections[inUse].code = code;
  sections[inUse].length = value.size();
  ++inUse;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const Data& value)
{
  if (inUse >= sections.size()) {
    return;
  }
  sections[inUse].data.append(value);
  sections[inUse].code = code;
  sections[inUse].length = value.size();
  ++inUse;
}

const ConcreteExtendedDataSectionV1&
ConcreteExtendedDataV1::getSection(std::size_t index) const
{
  return sections[index];
}

PayloadData
ConcreteExtendedDataV1::payload()
{
  if (inUse > 0) {
    PayloadData result;
    for (std::size_t i = 0;i < inUse;++i) {
      auto s = sections[i];
      result.append(s.code);
      result.append(s.length);
      result.append(s.data);
    }
    return result;
  }
  return PayloadData(); // empty
}

}
}
}
