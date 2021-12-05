//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_MODEL_H
#define HERALD_EXPOSURE_MODEL_H

#include "parameters.h"

#include "../datatype/date.h"
#include "../datatype/time_interval.h"

namespace herald {
namespace exposure {
  

template <typename... RiskModelTs>
class RiskModels {
public:
  static constexpr std::size_t Size = sizeof...(RiskModelTs);

  RiskModels(RiskModelTs&... riskModels)
    : models(std::array<
              std::variant<RiskModelTs...>
              ,Size
            >({std::variant<RiskModelTs...>(riskModels)...})
            )
  {}
  ~RiskModels() = default;
private:
  std::array<
    std::variant<RiskModelTs...>
    ,
    Size
  > models;
};



namespace model {

using namespace herald::datatype;
using namespace herald::exposure::parameter;

/**
 * @brief NOT FOR USE IN PRODUCTION - Sample Risk Model showing the use of two parameters over time
 * 
 * This model uses Luminosity and Proximity (any human, unconfirmed illness) as input variables, and age as a fixed risk parameter
 */
struct SampleDiseaseScreeningRiskModel {

  template <typename RiskParametersT, typename ExposureSourceT, typename RiskSinkT>
  bool produce(const RiskParametersT& riskParameters, const ExposureSourceT& exposures, const Date startTime, const Date endTime, const TimeInterval periodicity, RiskSinkT& sink) noexcept {
    // Create the start and end datetime values for each period of interest
    Exposure aggProx;
    Exposure aggLight;
    Age age;
    for (Date periodStart = startTime; periodStart < endTime;periodStart += periodicity) {
      Date periodEnd = periodStart + periodicity;

      // Now query the exposure manager for the variables we are interested in - aggregated as appropriate so we don't have to do it ourselves
      exposures.aggregate(herald::exposure::agent::humanProximity, periodStart, periodEnd, herald::analysis::aggregates::Sum, [aggProx&] (auto& Exposure cbValue) {
        aggProx = cbValue; // single value only
      });
      exposures.aggregate(herald::exposure::agent::lightBrightness, periodStart, periodEnd, herald::analysis::aggregates::Maximum, [aggLight&] (auto& Exposure cbValue) {
        aggLight = cbValue; // single value only
      });

      double confidence = 1.0;

      // Also fetch 'static' parameters, if available
      bool fetchedAgeOk = riskParameters.get(herald::exposure::parameter::age, age);
      if (!fetchedAgeOk) {
        age = 35; // Sensible default. E.g. population median age
        confidence -= 0.25;
      }

      // Now perform our calculation
      // WARNING - THIS IS A SAMPLE ONLY AND SHOULD NOT BE USED IN PRODUCTION!!!
      int multiplier = 1;
      if (aggLight > 0 && aggLight < 100) {
        // Risk increases with being indoors (poorly lit - a rough approximation, but simple as an example)
        multiplier = 2;
      }
      if (0 == aggLight) {
        // no light sample in this period
        confidence -= 0.25;
      }

      // Note the Sink reference has our internal IDs referenced within it, so no need for this struct to 'know' what instance it is
      sink.score(RiskScore{
        .periodStart = periodStart,
        .periodEnd = periodEnd,
        .value = multiplier * age * aggProx, // Risk scales linearly with age and RSSIMinute proximity score, and doubles if indoors vs outdoors
        .confidence = confidence
      });
    }
  }
};


}
}
}

#endif