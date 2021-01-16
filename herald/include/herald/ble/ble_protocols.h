//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef BLE_PROTOCOLS_H
#define BLE_PROTOCOLS_H

#include "../datatype/target_identifier.h"
#include "../engine/activities.h"

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::engine;

/**
 * The Herald protocols' low level activities. Implemented by each OS' provider
 */
class HeraldProtocolV1Provider {
public:
  HeraldProtocolV1Provider() = default;
  virtual ~HeraldProtocolV1Provider() = default;

  /** Opens a new connection. Returns true if successful or already connected */
  virtual bool openConnection(const TargetIdentifier& toTarget) = 0;
  /** Closes a connection. Returns true if successful or already disconnected */
  virtual bool closeConnection(const TargetIdentifier& toTarget) = 0;

  virtual void identifyOS(Activity, CompletionCallback) = 0;
  virtual void readPayload(Activity, CompletionCallback) = 0;
  virtual void immediateSend(Activity, CompletionCallback) = 0;
  virtual void immediateSendAll(Activity, CompletionCallback) = 0;
};

}
}

#endif