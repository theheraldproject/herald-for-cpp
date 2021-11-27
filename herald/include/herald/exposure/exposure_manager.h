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

// 'Hidden' internal default struct definitions
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

/**
 * @brief Acts as a delegate to be informed when a raw Analysis API value has changed
 * 
 * @tparam ModelT The type of data that has changed (E.g. RssiMinute)
 * @tparam EMT The (internally generated) Exposure Manager type (back reference)
 */
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

  /**
   * @brief Analysis API delegate callback function.
   * 
   * This method links the Exposure API to the Analysis API in Herald.
   * 
   * To link a custom exposure source to the Exposure API, use the ExposureManager.applyAdditionalExposure method, as used here.
   * 
   * @param sampled The Unique Identifier of the 'Sampled' object. Could be a remote person (E.g. Bluetooth Exposure), or temperature source, etc.
   * @param sample The new value to add to the list
   */
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
  /**
   * @brief Returns the number of active sources.
   * Always less than or equal to max_size
   * @return The current active size
   */
  const std::size_t sourceCount() const noexcept {
    return count;
  }

  /**
   * @brief Adds a new Agent source type, with appropriate SensorClass and Instance UUIDs.
   * 
   * @param agent The agent of exposure this source represents. E.g. radiation, sunlight, proximity
   * @param sensorClass The class of sensor. E.g. Gamma radition, single or quad channel light, or bluetooth/UWB proximity
   * @param instance The instance ID of a sensor (a device may have one or more sensors for each agent and sensorClass)
   * @return true If adding succeeded
   * @return false If adding failed (i.e. max_size has already been reached)
   */
  bool addSource(const Agent& agent, const SensorClass& sensorClass, const UUID& instance) noexcept {
    if (count >= max_size) {
      return false;
    }
    exposures[count] = ExposureArray<max_size>{
      ExposureMetadata{
        .agentId = agent, 
        .sensorClassId = sensorClass,
        .sensorInstanceId = instance
      }
    };
    ++count;
    return true;
  }

  /**
   * @brief Removes a source from the active list
   * 
   * @param instanceId The instance ID to remove
   * @return true If the instanceId is found and removed
   * @return false If the instanceId has already been removed
   */
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
  /**
   * @brief Generates an ExposureManagerDelegate appropriate to the given ExposureScoreT and ExposureManager instance
   */
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