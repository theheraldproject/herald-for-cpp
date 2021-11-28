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

  /**
   * @brief Represents an INTERNAL reference to a change that has occured. 
   * 
   * Used to efficiently cache change references until a notify action can be called.
   * 
   */
  struct ExposureChangeReference {
    UUID sensorInstanceId = UUID::unknown();
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
      count(0),
      changes(),
      changeCount(0),
      running(false)
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
  template <typename ModelT>
  bool addSource(const Agent& agent, const SensorClass& sensorClass, const UUID& instance) noexcept {
    if (count >= max_size) {
      return false;
    }
    exposures[count] = ExposureArray<max_size>{
      ExposureMetadata{
        .agentId = agent, 
        .sensorClassId = sensorClass,
        .sensorInstanceId = instance,
        .modelClassId = ModelT::modelClassId
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
    std::size_t pos = findMetaByInstanceId(instanceId);
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
    return running;
  }

  void enableRunning() noexcept {
    running = true;
  }

  void disableRunning() noexcept {
    running = false;
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

  /**
   * @brief Callback from multiple potential Analysis API callback handlers, passing the new value through.
   * 
   * @param sampled The identifier of the data type (E.g. temperature), or datatype-source (E.g. proximity-bluetoothId)
   * @param sample 
   */
  template <typename ModelT>
  void applyAdditionalExposure(const analysis::SampledID sampled, const analysis::Sample<ModelT> sample) noexcept {
    // Link to relevant Exposure by ModelT
    std::size_t pos = findMetaByModelClassId(ModelT::modelClassId);
    if (pos >= max_size) {
      return;
    }
    // Use SampledID as the instanceId for now (might not always be the case depending on source/model)
    auto& exposureArray = exposures[pos].contents();
    if (0 == exposureArray.size()) {
      exposureArray[0] = Exposure{
        .periodStart = sample.taken,
        .periodEnd = sample.taken,
        .value = (double)sample.value, // explicit conversion
        .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
      };
      // Record change for later notification
      recordChange(exposures[pos].getTag().sensorInstanceId, sample.taken, sample.taken);
    } else {
      // TODO Ensure split period is observed, rather than always using the first value
      // Append value to time period
      exposureArray[0] += Exposure{
        .periodStart = sample.taken,
        .periodEnd = sample.taken,
        .value = (double)sample.value, // explicit conversion
        .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
      };
      // Record change for later notification
      recordChange(exposures[pos].getTag().sensorInstanceId, exposureArray[0].periodStart, exposureArray[0].periodEnd);
    }
  }

  /**
   * @brief Notifies delegates of any changes that have occurred since the last time this method was called.
   * 
   * @return true If any notifications of changes occured
   * @return false If no notifications of changes occured
   */
  bool notifyOfChanges() noexcept {
    if (0 == changeCount) {
      return false;
    }
    bool anyNotified = false;
    for (std::size_t changeIndex = 0; changeIndex < changeCount; ++changeIndex) {
      std::size_t instancePos = findMetaBySensorInstanceId(changes[changeIndex].sensorInstanceId);
      if (instancePos < max_size) {
        anyNotified = true;
        handler.exposureLevelChanged(exposures[instancePos].getTag(), exposures[instancePos].contents()[0]); // TODO replace this with a safety check
      }
    }
    return anyNotified;
  }


  /// MARK: Methods invoked by external risk state classes (E.g. Exposure Notification frameworks)

private:
  CallbackHandlerT& handler;
  ExposureStoreT& store;

  /// /brief In memory ephemeral cached exposures
  ExposureSet<max_size> exposures;
  std::size_t count;

  std::array<ExposureChangeReference,max_size> changes;
  std::size_t changeCount;

  bool running;

  std::size_t findMeta(const ExposureMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (exposures[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaBySensorInstanceId(const UUID& sensorInstanceId) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (exposures[pos].getTag().sensorInstanceId == sensorInstanceId) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaByModelClassId(const UUID& modelClassId) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (exposures[pos].getTag().modelClassId == modelClassId) {
        return pos;
      }
    }
    return max_size;
  }

  void recordChange(const UUID& instanceId, const Date& periodStart, const Date& periodEnd) noexcept {
    if (!running) {
      return;
    }
    // TODO don't do this naively! Danger here!!!
    // TODO include start and end dates for efficiency
    changes[changeCount] = ExposureChangeReference{
      .sensorInstanceId = instanceId
    };
    ++changeCount;
  }
};

}
}

#endif