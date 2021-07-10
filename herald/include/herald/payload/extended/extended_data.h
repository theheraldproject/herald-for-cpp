//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXTENDED_DATA_H
#define HERALD_EXTENDED_DATA_H

#include "../../datatype/data.h"
#include "../../datatype/payload_data.h"
#include "../../datatype/payload_timestamp.h"

#include <optional>
#include <cstdint>

namespace herald {
namespace payload {
namespace extended {

using namespace herald::datatype;

using ExtendedDataSegmentCode = uint8_t;

// Abstract class only
class ExtendedData {
public:
  ExtendedData() = default;
  virtual ~ExtendedData() = default;

  virtual bool hasData() const = 0;
  virtual void addSection(ExtendedDataSegmentCode code, uint8_t value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, uint16_t value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, float value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, const std::string value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, const Data& value) = 0;

  virtual std::optional<PayloadData> payload() = 0;
};

// V1 Concrete types
// enum class ExtendedDataSegmentCodesV1 : ExtendedDataSegmentCode {
//   TextPremises = 0x10,
//   TextLocation = 0x11,
//   TextArea = 0x12,
//   LocationUrl = 0x13
// };

// operator ExtendedDataSegmentCode(const ExtendedDataSegmentCodesV1& from) {
//   return static_cast<uint8_t>(from);
// };
struct ExtendedDataSegmentCodesV1 {
  uint8_t value;

  constexpr ExtendedDataSegmentCodesV1(uint8_t v = 0) : value(v) {}
  constexpr operator uint8_t() const { return value; }

  static const ExtendedDataSegmentCodesV1 TextPremises, TextLocation, TextArea, LocationUrl;
};

constexpr const ExtendedDataSegmentCodesV1 
ExtendedDataSegmentCodesV1::TextPremises(0x10),
ExtendedDataSegmentCodesV1::TextLocation(0x11),
ExtendedDataSegmentCodesV1::TextArea(0x12),
ExtendedDataSegmentCodesV1::LocationUrl(0x13)
;

struct ConcreteExtendedDataSectionV1 {
  uint8_t code;
  uint8_t length;
  Data data;

  ConcreteExtendedDataSectionV1(uint8_t code,uint8_t length,const Data data)
    : code(code), length(length), data(data)
  {
    ;
  }

  ConcreteExtendedDataSectionV1(ConcreteExtendedDataSectionV1&& other)
    : code(other.code), length(other.length), data(std::move(other.data))
  {
    ;
  }

  ConcreteExtendedDataSectionV1(const ConcreteExtendedDataSectionV1& other)
    : code(other.code), length(other.length), data(other.data)
  {
    ;
  }

  ConcreteExtendedDataSectionV1& operator=(const ConcreteExtendedDataSectionV1& other) = default;
};

class ConcreteExtendedDataV1 : public ExtendedData {
public:
  ConcreteExtendedDataV1();
  ConcreteExtendedDataV1(const ConcreteExtendedDataV1& other); // copy ctor
  ConcreteExtendedDataV1(ConcreteExtendedDataV1&& other); // move ctor
  // ConcreteExtendedDataV1& operator=(const ConcreteExtendedDataV1& other); // copy assign
  // ConcreteExtendedDataV1& operator=(ConcreteExtendedDataV1&& other); // move assign
  ~ConcreteExtendedDataV1();

  // Overrides
  bool hasData() const override;
  void addSection(ExtendedDataSegmentCode code, uint8_t value) override;
  void addSection(ExtendedDataSegmentCode code, uint16_t value) override;
  void addSection(ExtendedDataSegmentCode code, float value) override;
  void addSection(ExtendedDataSegmentCode code, const std::string value) override;
  void addSection(ExtendedDataSegmentCode code, const Data& value) override;
  std::optional<PayloadData> payload() override;

  // V1 only methods
  const std::vector<ConcreteExtendedDataSectionV1>& getSections() const;

private:
  bool mHasData;
  std::vector<ConcreteExtendedDataSectionV1> sections;
};

}
}
}

#endif