//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef CONCRETE_PAYLOAD_DATA_SUPPLIER_H
#define CONCRETE_PAYLOAD_DATA_SUPPLIER_H

#include "payload_data_supplier.h"

#include <memory>

namespace herald {
namespace payload {

class ConcretePayloadDataSupplier : public PayloadDataSupplier {
public:
  ConcretePayloadDataSupplier() = default;
  ~ConcretePayloadDataSupplier() = default;

  // TODO implement OS concrete-specific functionality in CPP files

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // end namespace
} // end namespace

#endif