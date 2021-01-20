//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_CONTEXT_H
#define HERALD_CONTEXT_H

#include "ble/bluetooth_state_manager.h"

#include <iosfwd>
#include <string>
#include <memory>

namespace herald {

using namespace herald::ble;

namespace data {
class SensorLoggingSink; // fwd decl
}

using namespace herald::data;

/**
 * Some platforms require global configuration or static configuration that
 * doesn't map well on to C++ idioms. This class provides an extension capability
 * to allow this linking.
 */
class Context {
public:
  Context() = default;
  virtual ~Context() = default;

  virtual std::shared_ptr<SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor) = 0;
  virtual std::shared_ptr<BluetoothStateManager> getBluetoothStateManager() = 0;
};

/**
 * Default context that just sends logging to stdout
 */
class DefaultContext : public Context {
public:
  DefaultContext() = default;
  ~DefaultContext() = default;

  std::shared_ptr<BluetoothStateManager> getBluetoothStateManager() override;

  std::shared_ptr<SensorLoggingSink> getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor) override;
};

} // end namespace

#endif