//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "date.h"
#include "proximity.h"
#include "payload_data.h"

#include <string>

namespace herald {
namespace datatype {

class Encounter {
public:
  Encounter(Proximity didMeasure, PayloadData withPayload, Date timestamp);
  Encounter(Proximity didMeasure, PayloadData withPayload);
  Encounter(const std::string csvRow);
  ~Encounter();

  std::string csvString() const;

  bool isValid() const;

  const Proximity& proximity() const;
  const PayloadData& payload() const;
  const Date& timestamp() const;

private:
  class Impl;
  std::unique_ptr<Impl> mImpl; // PIMPL IDIOM
};


} // end namespace
} // end namespace

#endif