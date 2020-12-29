//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "payload/extended/extended_data.h"
#include "datatype/data.h"

#include <vector>
#include <string>
#include <cstddef>

namespace herald {
namespace payload {
namespace extended {

class ConcreteExtendedDataV1::Impl {
public:
  Impl();
  ~Impl();

  bool hasData;
  std::vector<ConcreteExtendedDataSectionV1> sections;
};

ConcreteExtendedDataV1::Impl::Impl()
  : hasData(false), sections()
{
  ;
}

ConcreteExtendedDataV1::Impl::~Impl()
{
  ;
}



constexpr const ExtendedDataSegmentCodesV1 
ExtendedDataSegmentCodesV1::TextPremises(0x10),
ExtendedDataSegmentCodesV1::TextLocation(0x11),
ExtendedDataSegmentCodesV1::TextArea(0x12),
ExtendedDataSegmentCodesV1::LocationUrl(0x13)
;


ConcreteExtendedDataV1::ConcreteExtendedDataV1()
  : mImpl(std::make_unique<Impl>())
{
  ;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(const ConcreteExtendedDataV1& other)
  : mImpl(std::make_unique<Impl>())
{
  mImpl->hasData = other.mImpl->hasData;
  mImpl->sections = other.mImpl->sections;
}

ConcreteExtendedDataV1::ConcreteExtendedDataV1(ConcreteExtendedDataV1&& other)
  : mImpl(std::move(other.mImpl))
{
  ;
}

// ConcreteExtendedDataV1&
// ConcreteExtendedDataV1::operator=(const ConcreteExtendedDataV1& other)
// {
//   mImpl->hasData = other.mImpl->hasData;
//   mImpl->sections = other.mImpl->sections;

//   return *this;
// }

// ConcreteExtendedDataV1&
// ConcreteExtendedDataV1::operator=(ConcreteExtendedDataV1&& other)
// {
//   mImpl->hasData = other.mImpl->hasData;
//   mImpl->sections = std::move(other.mImpl->sections);

//   return *this;
// }

ConcreteExtendedDataV1::~ConcreteExtendedDataV1()
{
  ;
}



bool
ConcreteExtendedDataV1::hasData() const
{
  return mImpl->hasData;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint8_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value));
  mImpl->sections.emplace_back(code, 1, std::move(d));
  mImpl->hasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint16_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value >> 8));
  d.push_back(std::byte(value & 0xff));
  mImpl->sections.emplace_back(code, 1, std::move(d));
  mImpl->hasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, float value)
{
  std::vector<std::byte> d;
  for (std::size_t i = sizeof(float);i > 0 ;--i) {
    d.push_back(std::byte(((std::size_t)value) >> (8 * (i - 1))));
  }
  mImpl->sections.emplace_back(code,sizeof(float), std::move(d));
  mImpl->hasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const std::string value)
{
  std::vector<std::byte> d;
  for (auto c : value) {
    d.push_back(std::byte(c));
  }
  mImpl->sections.emplace_back(code, value.size(), std::move(d));
  mImpl->hasData = true;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const Data& value)
{
  mImpl->sections.emplace_back(code, value.size(), value);
  mImpl->hasData = true;
}

const std::vector<ConcreteExtendedDataSectionV1>&
ConcreteExtendedDataV1::getSections() const
{
  return mImpl->sections;
}

std::optional<PayloadData>
ConcreteExtendedDataV1::payload()
{
  if (mImpl->hasData) {
    PayloadData result;
    for (auto s : mImpl->sections) {
      result.append(s.code);
      result.append(s.length);
      result.append(s.data);
    }
    return result;
  }
  return std::optional<PayloadData>(); // empty optional
}

}
}
}
