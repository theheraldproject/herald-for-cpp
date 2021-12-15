//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_MANAGER_H
#define HERALD_EXPOSURE_MANAGER_H

#include "../datatype/exposure.h"
#include "../datatype/time_interval.h"
#include "../datatype/date.h"

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
    Date periodStart = Date{0};
    Date periodEnd = Date{0};

    bool operator==(const ExposureChangeReference& other) const noexcept {
      return sensorInstanceId == other.sensorInstanceId;
    }
    bool operator!=(const ExposureChangeReference& other) const noexcept {
      return sensorInstanceId != other.sensorInstanceId;
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

  ExposureManagerDelegate& operator=(ExposureManagerDelegate&& toMove) noexcept {
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
     changes(),
     anchor(Date{}), // Default to instantiation DateTime
     period(TimeInterval::hours(24)), // Default to one day interval
     running(false)
  {
    ;
  }

  // Once created, we must always be referenced - never copied
  ExposureManager(const ExposureManager&) = delete;
  ExposureManager(ExposureManager&&) = delete;

  ~ExposureManager() noexcept = default;

  /// MARK: Manager configuration methods
  /**
   * @brief Returns the number of active sources.
   * Always less than or equal to max_size
   * @return The current active size
   */
  const std::size_t sourceCount() const noexcept {
    return exposures.size();
  }

  bool setGlobalPeriodInterval(const Date recentAnchor, const TimeInterval periodSize) noexcept {
    if (running) {
      return false; // guard against mid-work modification
    }
    anchor = recentAnchor; // Do we handle anchor time that is in the future? No - could be a future simulation
    period = periodSize;
    return true;
  }

  const Date getGlobalPeriodAnchor() const noexcept {
    return anchor;
  }

  const TimeInterval getGlobalPeriodInterval() const noexcept {
    return period;
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
    if (exposures.size() >= max_size) {
      return false;
    }
    // TODO check if we've already entered this source
    exposures.add(ExposureArray<max_size>{
      ExposureMetadata{
        .agentId = agent, 
        .sensorClassId = sensorClass,
        .sensorInstanceId = instance,
        .modelClassId = ModelT::modelClassId
      }
    });
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
    std::size_t pos = findMetaBySensorInstanceId(instanceId);
    if (pos >= max_size) {
      // agent not found - return false
      return false;
    }
    auto count = exposures.size();
    for (std::size_t mp = pos;mp < count - 1;++mp) {
      exposures[mp] = exposures[mp + 1];
    }
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
    applyAdditionalExposure(ModelT::modelClassId,sampled,sample.taken,(double)sample.value);
  }

  /**
   * @brief Notifies delegates of any changes that have occurred since the last time this method was called.
   * 
   * @return true If any notifications of changes occured
   * @return false If no notifications of changes occured
   */
  bool notifyOfChanges() noexcept {
    if (0 == changes.size()) {
      return false;
    }
    bool anyNotified = false;
    for (std::size_t changeIndex = 0; changeIndex < changes.size(); ++changeIndex) {
      std::size_t instancePos = findMetaBySensorInstanceId(changes[changeIndex].sensorInstanceId);
      if (instancePos < max_size) {
        anyNotified = true;
        auto exposureIter = exposures[instancePos].contents().begin();
        exposureIter += instancePos; // advance to the position of interest to the receiver
        auto exposureEnd = exposures[instancePos].contents().end();
        handler.exposureLevelChanged(exposures[instancePos].getTag(), exposureIter, exposureEnd); //exposures[instancePos].contents()[0]); // TODO replace this with a safety check
        // TODO fire the above for only those items that have changed, not the whole array or first element, using periodStart and periodEnd in the change metadata
      }
    }
    // reset changes for the next run
    changes.clear();
    return anyNotified;
  }


  /// MARK: Methods used by querying external classes (E.g. Risk Scoring algorithms)

  /**
   * @brief Recursively calls the given callable for each Exposure with the requisite Agent within the given time bounds.
   * 
   * @tparam AggT The aggregate type to apply (E.g. an Analysis API or custom aggregation)
   * @tparam CallableT The callable (E.g. Lambda) to call for each aggregated result
   * @param agent The agent of interest
   * @param periodStart Earliest time we're interested in (inclusive of overlaps)
   * @param periodEnd Most recent time we're interested in (inclusove of overlaps)
   * @param agg The aggregate to apply to the matching Exposures (from Analysis API, or custom)
   * @param c The callable to call - once or multiple times depending on the output of the aggregate
   */
  template <typename AggT, typename CallableT>
  void aggregate(const Agent& agent, const Date& periodStart, const Date& periodEnd, 
    AggT&& agg, CallableT c) {
      //call [&aggLight] (Exposure cbValue) {
    // TODO fill out this method
  }


  /// MARK: Methods invoked by external risk state classes (E.g. Exposure Notification frameworks)

  /**
   * @brief Get the Count of the number of exposure scores recorded for a given instance ID
   * 
   * @param sensorInstanceId The sensor instanceId we are interested in
   * @return std::size_t The number of current, in memory, exposure period values stored
   */
  std::size_t getCountByInstanceId(const UUID& sensorInstanceId) const noexcept {
    std::size_t pos = findMetaBySensorInstanceId(sensorInstanceId);
    if (pos >= max_size) {
      // not found - return false
      return 0;
    }
    auto sz = exposures[pos].contents().size();
    return sz;
  }

private:
  CallbackHandlerT& handler;
  ExposureStoreT& store;

  /// \brief In memory ephemeral cached exposures
  ExposureSet<max_size> exposures;

  AllocatableArray<ExposureChangeReference,max_size> changes;

  Date anchor;
  TimeInterval period;

  bool running;

  std::size_t findMeta(const ExposureMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      if (exposures[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaBySensorInstanceId(const UUID& sensorInstanceId) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      auto& exp = exposures[pos];
      auto& t = exp.getTag();
      auto& siid = t.sensorInstanceId;
      if (siid == sensorInstanceId) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaByModelClassId(const UUID& modelClassId) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      auto& mcid = exposures[pos].getTag().modelClassId;
      if (mcid == modelClassId) {
        return pos;
      }
    }
    return max_size;
  }
  
  void applyAdditionalExposure(const UUID& modelId, const analysis::SampledID sampled, const Date& sampleTaken, const double sampleValue) noexcept {
    std::size_t pos = findMetaByModelClassId(modelId);
    if (pos >= max_size) {
      return;
    }
    // Use SampledID as the instanceId for now (might not always be the case depending on source/model)
    auto& exposureArray = exposures[pos].contents();
    if (0 == exposureArray.size()) {
      exposureArray.add(Exposure{
        .periodStart = sampleTaken,
        .periodEnd = sampleTaken,
        .value = sampleValue, // explicit conversion
        .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
      });
      // Record change for later notification
      recordChange(exposures[pos].getTag().sensorInstanceId, sampleTaken, sampleTaken);
    } else {
      // Ensure split by period is observed
      // TODO take into account of anchor time, rather than always rely upon startTime of previous element (which may not align exactly with anchor time + n*period)
      // get last exposure
      auto& last = exposureArray[exposureArray.size() - 1];
      // determine if last start time plus this time is greater than window size
      if (last.periodStart + period <= sampleTaken) {
        // if so, set endTime to window size, and create new element
        last.periodEnd = last.periodStart + period;
        exposureArray.add(Exposure{
          .periodStart = (last.periodStart + period) >= sampleTaken ? (last.periodStart + period) : sampleTaken, // set to end time of previous period, to be sure, unless we've skipped 1 or more windows
          .periodEnd = sampleTaken,
          .value = sampleValue, // explicit conversion
          .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
        });
        // Record change for later notification
        recordChange(exposures[pos].getTag().sensorInstanceId, last.periodStart, sampleTaken);
      } else {
        // if not, append
        // Append value to time period
        exposureArray[exposureArray.size() - 1] += Exposure{
          .periodStart = sampleTaken,
          .periodEnd = sampleTaken,
          .value = sampleValue, // explicit conversion
          .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
        };
        // Record change for later notification
        recordChange(exposures[pos].getTag().sensorInstanceId, exposureArray[0].periodStart, exposureArray[0].periodEnd);
      }
    }
  }

  void recordChange(const UUID& instanceId, const Date& periodStart, const Date& periodEnd) noexcept {
    if (!running) {
      return;
    }
    // Note including start and end dates for efficiency
    // Check for an existing change for this instanceId, and modify its start and end times
    std::size_t cpos = max_size;
    for (std::size_t i = 0;i < changes.size();++i) {
      if (changes[i].sensorInstanceId == instanceId) {
        cpos = i;
      }
    }
    if (max_size != cpos) {
      changes[cpos].periodStart = periodStart < changes[cpos].periodStart ? periodStart : changes[cpos].periodStart;
      changes[cpos].periodEnd = periodEnd > changes[cpos].periodEnd ? periodEnd : changes[cpos].periodEnd;
    } else {
      changes.add(ExposureChangeReference{
        .sensorInstanceId = instanceId,
        .periodStart = periodStart,
        .periodEnd = periodEnd
      });
      // ++changeCount;
    }
  }
};

}
}

#endif