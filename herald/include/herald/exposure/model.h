//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_MODEL_H
#define HERALD_EXPOSURE_MODEL_H

#include "parameters.h"

#include "../datatype/exposure_risk.h"
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

  /// MARK: Callable convenience methods (hides variant implementation)

  template <typename AlgoCallbackT>
  void forMatchingAlgorithm(AlgoCallbackT callback, UUID algorithmId) noexcept {
    // Loop over model types and find one with the same algorithmId
    bool found = false;
    for (auto& modelVarRef: models) {
      std::visit([&algorithmId,&callback,&found] (auto&& algo) {
        if (((decltype(algo))algo).algorithmId == algorithmId) {
          found = true;
          callback((decltype(algo))algo);
        }
      }, modelVarRef);
    }
  }

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
  // ID to enable dynamic linking at runtime to static compiled risk model
  static constexpr Agent algorithmId{1};
    
  //   UUID::data_type{
  //   { // passes the data as an initialiser list to std::array
  //     0x00,0x00,0x00,0x00,
  //     0x00,0x00,
  //     0x00,0x00, // The UUID constructor will set the v4 UUID fields for me
  //     0x00,0x00,
  //     0x00,0x00,0x00,0x00,0x00,0x01 /* Last byte is 1 for me... */
  //   }
  // }}; // WARNING: Generate a v4 UUID online, check it isn't used, and place it here for your class

  // TODO later refine by also including start/end time and periodicity - incase an agent is only relevant at a particular time (MAY need ALL agents in that call though...)
  bool potentiallyDirty(const Agent& agent, const Exposure& exposure) const noexcept {
    return (
      (agent == herald::datatype::agent::humanProximity) ||
      (agent == herald::datatype::agent::lightBrightness)
    );
  }

  template <typename RiskParametersT, typename ExposureSourceT, typename RiskSinkT>
  bool produce(const RiskParametersT& riskParameters, const ExposureSourceT& exposures, const Date startTime, const Date endTime, const TimeInterval periodicity, RiskSinkT& sink) noexcept {
    // Create the start and end datetime values for each period of interest
    Exposure aggProx;
    Exposure aggLight;
    Age age;
    double rawAge;
    for (Date periodStart = startTime; periodStart < endTime;periodStart += periodicity) {
      Date periodEnd = periodStart + periodicity;

      // Now query the exposure manager for the variables we are interested in - aggregated as appropriate so we don't have to do it ourselves
      exposures.aggregate(herald::datatype::agent::humanProximity, periodStart, periodEnd, herald::analysis::aggregates::Sum{}, [&aggProx] (Exposure cbValue) {
        aggProx = cbValue; // single value only
      });
      exposures.aggregate(herald::datatype::agent::lightBrightness, periodStart, periodEnd, herald::analysis::aggregates::Maximum{}, [&aggLight] (Exposure cbValue) {
        aggLight = cbValue; // single value only
      });

      double confidence = 1.0;

      // Also fetch 'static' parameters, if available
      bool fetchedAgeOk = riskParameters.get(herald::exposure::parameter::age, rawAge);
      if (!fetchedAgeOk) {
        age = 35; // Sensible default. E.g. population median age
        confidence -= 0.25;
      } else {
        age = (std::uint8_t)rawAge;
      }

      // Now perform our calculation
      // WARNING - THIS IS A SAMPLE ONLY AND SHOULD NOT BE USED IN PRODUCTION!!!
      int multiplier = 1;
      if (aggLight.value > 0 && aggLight.value < 100) {
        // Risk increases with being indoors (poorly lit - a rough approximation, but simple as an example)
        multiplier = 2;
      }
      if (0 == aggLight.value) {
        // no light sample in this period
        confidence -= 0.25;
      }

      // Note the Sink reference has our internal IDs referenced within it, so no need for this struct to 'know' what instance it is
      // TODO add identification for target agent, algorithmID, and algorithmInstanceID
      sink.score(RiskScore{
        .periodStart = periodStart,
        .periodEnd = periodEnd,
        .value = multiplier * age * aggProx.value, // Risk scales linearly with age and RSSIMinute proximity score, and doubles if indoors vs outdoors
        .confidence = confidence
      });
    }
    return true; // all worked well
  }
};


}
}
}

#endif