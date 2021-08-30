//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include "../datatypes/presence.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace heraldns {
namespace providers {

using namespace heraldns::datatype;

class TransmissionModelProvider {
public:
  TransmissionModelProvider() = default;
  virtual ~TransmissionModelProvider() = default;

  virtual void initialiseInfectionState(std::shared_ptr<Presence> presence) = 0;
  virtual void determineInfectionState(std::shared_ptr<Presence> presence, double minutesPassed, 
    uint64_t tick) = 0;
};

}
}

#endif