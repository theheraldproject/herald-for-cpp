//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "payload/extended/extended_data.h"
#include "datatype/data.h"

#include <vector>
#include <string>

namespace herald {
namespace payload {
namespace extended {

class ConcreteExtendedDataV1::Impl {
public:
  Impl();
  ~Impl() = default;

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




ConcreteExtendedDataV1::ConcreteExtendedDataV1()
  : mImpl(std::make_unique<Impl>())
{
  ;
}


ConcreteExtendedDataV1::~ConcreteExtendedDataV1()
{
  ;
}



bool
ConcreteExtendedDataV1::hasData() const
{
  return mImpl->m_hasData;
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint8_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value));
  mImpl->sections.emplace_back({code, 1, std::move(d)});
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, uint16_t value)
{
  std::vector<std::byte> d;
  d.push_back(std::byte(value >> 8));
  d.push_back(std::byte(value & 0xff));
  mImpl->sections.emplace_back({code, 1, std::move(d)});
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, float_t value)
{
  std::vector<std::byte> d;
  for (std::size_t i = sizeof(float_t);i > 0 ;--i) {
    d.push_back(std::byte(value >> (8 * (i - 1))));
  }
  mImpl->sections.emplace_back({code, std::move(d)});
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const std::string value)
{
  std::vector<std::byte> d;
  for (auto c : value) {
    d.push_back(std::byte(c));
  }
  mImpl->sections.emplace_back({code, std::move(d)});
}

void
ConcreteExtendedDataV1::addSection(ExtendedDataSegmentCode code, const Data& value)
{
  mImpl->sections.emplace_back({code, value});
}

const std::vector<ConcreteExtendedDataSectionV1>& getSections() const
{
  return sections;
}


}
}
}
