//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_RISK_MANAGER_H
#define HERALD_RISK_MANAGER_H

#include "parameters.h"
#include "model.h"

#include "../datatype/exposure.h"
#include "../datatype/time_interval.h"
#include "../datatype/date.h"

#include <functional>
#include <optional>

namespace herald {
namespace exposure {

template <typename RiskModelsT, typename RiskParametersT>
class RiskManager {
public:
  RiskManager(RiskModelsT&& riskModelsToOwn, RiskParametersT&& riskParametersToOwn)
   : models(std::move(riskModelsToOwn)),
     parameters(std::move(riskParametersToOwn))
  {
    ;
  }

  ~RiskManager() = default;

  // TODO member functions here

private:
  RiskModelsT models;
  RiskParametersT parameters;
};


}
}

#endif