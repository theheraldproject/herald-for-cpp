//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_RECEIVER_H
#define BLE_RECEIVER_H

#include "../sensor.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// Tagging interface
class BLEReceiver : public Sensor {
public:
  BLEReceiver() = default;
  virtual ~BLEReceiver() = default;

  // BLEReceiver specific methods
  virtual bool immediateSend(Data data, const TargetIdentifier& targetIdentifier) = 0;
  virtual bool immediateSendAll(Data data) = 0;
  
  // Remaining methods inherited as pure virtual from Sensor class
};

} // end namespace
} // end namespace

#endif