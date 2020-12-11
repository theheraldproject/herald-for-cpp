//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_TRANSMITTER_H
#define BLE_TRANSMITTER_H

#include "../sensor.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

// Tagging interface
/**
 * Beacon transmitter broadcasts a fixed service UUID to enable background scan by iOS. When iOS
 * enters background mode, the UUID will disappear from the broadcast, so Android devices need to
 * search for Apple devices and then connect and discover services to read the UUID.
 */
class BLETransmitter : public Sensor {
public:
  BLETransmitter() = default;
  virtual ~BLETransmitter() = default;

  // BLETransmitter specific methods
  /**
   * Get current payload.
   */
  PayloadData payloadData() const;

  /**
   * Is transmitter supported.
   *
   * @return True if BLE advertising is supported.
   */
  bool isSupported() const;

  // Remaining methods inherited as pure virtual from Sensor class
};

} // end namespace
} // end namespace

#endif