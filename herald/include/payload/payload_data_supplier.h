//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef PAYLOAD_DATA_SUPPLIER_H
#define PAYLOAD_DATA_SUPPLIER_H

#include "../device.h"

namespace herald {
namespace payload {

class PayloadDataSupplier {
public:
  PayloadDataSupplier() = default;
  virtual ~PayloadDataSupplier() = default;

  virtual std::optional<PayloadData> legacyPayload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device) = 0;
  virtual std::optional<PayloadData> payload(const PayloadTimestamp timestamp, const std::shared_ptr<Device> device) = 0;
  virtual std::vector<PayloadData> payload(const Data& data) = 0;
};

} // end namespace
} // end namespace

#endif