//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_MANAGER_H
#define HERALD_EXPOSURE_MANAGER_H

#include "../datatype/exposure.h"

#include <functional>
#include <optional>

namespace herald {
namespace exposure {
  
/**
 * /brief Uses the Analysis API in Herald to observe sensor readings and turn them into
 * aggregated exposure scores.
 *
 * This class also responds to external modifications to exposure confidence. E.g.
 * turning a general proximity exposure score into a "confirmed" COVID-19 exposure score.
 *
 * As an example, as I walk about I may have 0.9 (near full confidence) in a proximity score
 * but I do not convert that to a COVID-19 exposure until someone becomes symptomatic or is tested.
 * Should this occur, I could use the exposure scores from proximity to generate COVID-19 exposure
 * by aggregating sets of scores from different people with different levels of confidence.
 * If someone is symptomatic I may set COVID-19 (NOT raw proximity) confidence to 0.2, when they
 * have a positive lateral flow test perhaps 0.67 (approx the true positive rate of LFTs) and when a PCR
 * test is positive, to 0.98. This class manages the complex exposure scoring behind this.
 * 
 * This allows the related Risk Score algorithms to deal with all exposures in the same manner - 
 * that is, summary scores of exposure to 'Agents' over specific periods of time.
 */
template <typename CallbackHandlerT, 
  std::size_t MaxInMemoryExposureSummaries, typename ExposureStoreT>
class ExposureManager;
// FWD Declare

namespace {
  struct DefaultDevNullExposureStore {
  };

  struct DefaultNullExposureCallbackHandler {
    void exposureChanged(const ExposureMetadata& meta,
      const Exposure& changed) noexcept
    {
      ;
    }
  };
}
using DefaultExposureManager = ExposureManager<DefaultNullExposureCallbackHandler,8,DefaultDevNullExposureStore>;

template <typename ModelT, typename EMT>
class ExposureManagerDelegate {
public:
  using value_type = ModelT;
  using manager_type = EMT;
  
  ExposureManagerDelegate() noexcept
   : manager({})
  {
    ;
  }
  
  ExposureManagerDelegate(ExposureManagerDelegate& toMove) noexcept
   : manager(toMove.manager)
  {
    ;
  }
  
  ExposureManagerDelegate(const ExposureManagerDelegate& toCopy) noexcept
   : manager(toCopy.manager)
  {
    ;
  }

  ExposureManagerDelegate(EMT& managerRef) noexcept
   : manager(managerRef)
  {
    ;
  }

  ~ExposureManagerDelegate() noexcept = default;

  ExposureManagerDelegate& operator=(ExposureManagerDelegate& toMove) noexcept {
    manager = toMove.manager;
    return *this;
  }

  ExposureManagerDelegate& operator=(const ExposureManagerDelegate& toCopy) noexcept {
    manager = toCopy.manager;
    return *this;
  }

  // Analysis Delegate Manager callback method
  void newSample(herald::analysis::SampledID sampled, herald::analysis::Sample<ModelT> sample) noexcept {
    // Check if we're adding risk exposure currently on this device
    if (!manager.has_value() || !manager.value().get().isRunning()) {
      return;
    }
    // If so, find associated Agents for this exposure model, and increment risk value
    manager.value().get().applyAdditionalExposure(sampled,sample);
  }

private:
  std::optional<std::reference_wrapper<EMT>> manager;
};



// Now the full declaration (was fwd decl earlier)
template <typename CallbackHandlerT, 
  std::size_t MaxInMemoryExposureSummaries, typename ExposureStoreT>
class ExposureManager {
public:
  static constexpr std::size_t max_size = MaxInMemoryExposureSummaries;

  ExposureManager(CallbackHandlerT& initialHandler, ExposureStoreT& initialExposureStore) noexcept
    : handler(initialHandler),
      store(initialExposureStore),
      exposures(),
      count()
  {
    ;
  }

  ~ExposureManager() noexcept = default;

  /// MARK: Manager configuration methods
  const std::size_t sourceCount() const noexcept {
    return count;
  }

  bool addSource(const Agent& agent, const SensorClass& sensorClass, const UUID& instance) noexcept {
    if (count >= max_size) {
      return false;
    }
    exposures[count] = ExposureArray{
      ExposureMetadata{
        .agentId = agent, 
        .sensorClassId = sensorClass,
        .sensorInstanceId = instance
      }
    };
    ++count;
    return true;
  }

  bool removeSource(const UUID& instanceId) noexcept {
    std::size_t pos = findMeta(instanceId);
    if (pos >= max_size) {
      // agent not found - return false
      return false;
    }
    for (std::size_t mp = pos;mp < count - 1;++mp) {
      exposures[mp] = exposures[mp + 1];
    }
    --count;
    return true;
  }

  const bool isRunning() const noexcept {
    return false;
  }

  /// MARK: Event driven methods

  template <typename ExposureScoreT>
  ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT, max_size, ExposureStoreT>>
  analysisDelegate() noexcept {
    return ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT, max_size, ExposureStoreT>>(
      *this
    );
  }


  /// MARK: Methods invoked by external risk state classes (E.g. Exposure Notification frameworks)

private:
  CallbackHandlerT& handler;
  ExposureStoreT& store;

  /// /brief In memory ephemeral cached exposures
  ExposureSet<max_size> exposures;
  std::size_t count;

  std::size_t findMeta(const ExposureMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (exposures[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMeta(const UUID& sensorInstanceId) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (exposures[pos].getTag().sensorInstanceId == sensorInstanceId) {
        return pos;
      }
    }
    return max_size;
  }
};

}
}

#endif