//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef EXTENDED_DATA_H
#define EXTENDED_DATA_H

#include "../../datatype/data.h"
#include "../../datatype/payload_data.h"

#include <optional>

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
  virtual void addSection(ExtendedDataSegmentCode code, float_t value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, const std::string value) = 0;
  virtual void addSection(ExtendedDataSegmentCode code, const Data& value) = 0;

  virtual std::optional<PayloadData> payload() = 0;
};

// V1 Concrete types
enum class ExtendedDataSegmentCodesV1 : uint8_t {
  TextPremises = 0x10,
  TextLocation = 0x11,
  TextArea = 0x12,
  LocationUrl = 0x13
};

struct ConcreteExtendedDataSectionV1 {
  uint8_t code;
  uint8_t length;
  Data data;
};

class ConcreteExtendedDataV1 : public ExtendedData {
public:
  ConcreteExtendedDataV1();
  ~ConcreteExtendedDataV1();

  // Overrides
  bool hasData() const override;
  void addSection(ExtendedDataSegmentCode code, uint8_t value) override;
  void addSection(ExtendedDataSegmentCode code, uint16_t value) override;
  void addSection(ExtendedDataSegmentCode code, float_t value) override;
  void addSection(ExtendedDataSegmentCode code, const std::string value) override;
  void addSection(ExtendedDataSegmentCode code, const Data& value) override;

  // V1 only methods
  const std::vector<ConcreteExtendedDataSectionV1>& getSections() const;

private:
  class Impl; // fwd decl
  std::unique_ptr<Impl> mImpl;
};

}
}
}

#endif