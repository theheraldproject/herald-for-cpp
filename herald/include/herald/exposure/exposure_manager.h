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
 * @brief A basic in-memory store of Exposure information
 * 
 * This class is intended to be used standalone, and does not contain memory cache logic.
 * 
 * @tparam MaxInMemoryExposureSummaries The maximum number of Exposure Source arrays to maintain in RAM.
 */
template <std::size_t MaxInMemoryExposureSummaries>
class FixedMemoryExposureStore {
public:
  /**
   * @brief The maximum number of exposure sources this exposure store supports
   * 
   */
  static constexpr std::size_t max_size = MaxInMemoryExposureSummaries;

  /**
   * @brief Construct a new Fixed Memory Exposure Store object.
   * Default constructor
   */
  FixedMemoryExposureStore()
   : exposures()
  {
    ;
  }

  /// MARK: Exposure array and elements access and size methods

  /**
   * @brief Provisions space, if available, for the given ExposureMetadata
   * 
   * @param meta The ExposureMetadata to provision storage for
   * @return true If the metadata already has storage provisioned, or if provisioning of new storage succeeded
   * @return false If storage could not be provisioned for this exposure metadata
   */
  bool add(ExposureMetadata meta) noexcept {
    // TODO check if we already have provisioned storage for this metadata description
    if (exposures.size() >= max_size) {
      return false;
    }
    return exposures.add(
      ExposureArray<max_size>{
      meta
    });
  }

  // TODO add methods for adding/removing individual exposure values to the array created in add(), above

  /**
   * @brief Removes the whole set of exposure information for the given ExposureMetadata::instanceId value
   * 
   * @param instanceId The Instance ID (UUID) of the ExposureMetadata to remove from storage.
   * @return true If the instanceId was found (and thus removed)
   * @return false If the instanceId was not found
   */
  bool remove(const UUID& instanceId) noexcept {
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

  /**
   * @brief Returns the number of currently allocated ExposureMetadata arrays
   * 
   * @return const std::size_t The number of arrays currently used. Always <= max_size.
   */
  const std::size_t size() const noexcept {
    return exposures.size();
  }

  /**
   * @brief Get the Contents for the given exposure position
   * 
   * WARNING: does not validate the pos value passed to it yet.
   * 
   * @param pos Position of the allocated exposure metadata to return the ExposureArray for
   * @return auto& The contents Array (ExposureArray instance)
   */
  auto& getContents(std::size_t pos) noexcept {
    return exposures[pos].contents();
  }

  /**
   * @brief Get the Tag object for the ExposureMetadata instance at the given pos position
   * 
   * WARNING: does not validate the pos value passed to it yet.
   * 
   * @param pos Position of the allocated exposure metadata to return the tag for
   * @return auto& The Tag (ExposureMetadata instance) at the given position
   */
  auto& getTag(std::size_t pos) noexcept {
    return exposures[pos].getTag();
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
   * @param c The callable to call - once or multiple times depending on the output of the aggregate with signature  (const Exposure& cbValue) -> void
   */
  template <typename AggT, typename CallableT>
  void aggregate(const Agent& agent, const Date& periodStart, const Date& periodEnd, 
    AggT&& agg, CallableT callable) const noexcept {
    // TODO validate why we need to be const as a method (not always reasonable if data has to be shunted to/from memory)

    // Find our exposure scores for this agent
    // Reset aggregator (just to be safe)
    agg.reset();
    agg.beginRun(1); // TODO support multi-pass aggregations
    
    // Loop through until we find a score that overlaps (i.e. is not entirely within the exact time period) with that requested
    // TODO handle the situation where multiple sensor instances produce the same agent (Note: We may already aggregate this in the manager... Check...)
    auto pos = findMetaByAgentId(agent);
    if (max_size != pos) {
      // Pass the value(s) to the aggregator
      auto iter = exposures[pos].ccontents().cbegin();
      auto end = exposures[pos].ccontents().cend();
      for (; iter != end;++iter) {
        const auto& score = *iter;
        agg.map(score.value);
      }
      // TODO support aggregators that produce more than one value
      
      // For each (likely single) output, call the callable
      callable(Exposure{
        .periodStart = periodStart,
        .periodEnd = periodEnd,
        .value = agg.reduce(),
        .confidence = 1.0 // TODO get this from the aggregator itself
      });
    }
  }


  /// MARK: Search methods

  /**
   * @brief Returns the position of the given ExposureMetadata (by getTag()::operator==() )
   * 
   * @param meta The ExposureMetadata to search for
   * @return std::size_t The position of the ExposureMetadata, or max_size if not found
   */
  std::size_t findMeta(const ExposureMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      if (exposures[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  /**
   * @brief Returns the position of the given ExposureMetadata (by sensorInstanceId )
   * 
   * @param sensorInstanceId The ExposureMetadata::sensorInstanceId (UUID) to search for
   * @return std::size_t The position of the ExposureMetadata, or max_size if not found
   */
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

  /**
   * @brief Returns the position of the given ExposureMetadata (by agentId )
   * 
   * @param agent The ExposureMetadata::agentId (Agent aka UUID) to search for
   * @return std::size_t The position of the ExposureMetadata, or max_size if not found
   */
  std::size_t findMetaByAgentId(const Agent& agent) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      auto& exp = exposures[pos];
      auto& t = exp.getTag();
      auto& siid = t.agentId;
      if (siid == agent) {
        return pos;
      }
    }
    return max_size;
  }

  /**
   * @brief Returns the position of the given ExposureMetadata (by modelClassId )
   * 
   * @param modelClassId The ExposureMetadata::modelClassId (UUID) to search for
   * @return std::size_t The position of the ExposureMetadata, or max_size if not found
   */
  std::size_t findMetaByModelClassId(const UUID& modelClassId) const noexcept {
    for (std::size_t pos = 0;pos < exposures.size();++pos) {
      const auto& mcid = exposures[pos].getTag().modelClassId;
      if (mcid == modelClassId) {
        return pos;
      }
    }
    return max_size;
  }

  template <typename CallableT>
  void over(std::size_t pos, CallableT callable) noexcept {
    auto iter = exposures[pos].ccontents().cbegin();
    auto end = exposures[pos].ccontents().cend();
    for (;iter != end;++iter) {
      const auto& exposure = *iter;
      callable(exposure);
    }
  }

private:
  /// \brief In memory ephemeral cached exposures
  ExposureSet<max_size> exposures;
};





  
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
  typename ExposureStoreT>
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
using DefaultExposureManager = ExposureManager<DefaultNullExposureCallbackHandler,DefaultDevNullExposureStore>;

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
  typename ExposureStoreT>
class ExposureManager {
public:
  ExposureManager(CallbackHandlerT& initialHandler, ExposureStoreT& initialExposureStore) noexcept
   : handler(initialHandler),
     store(initialExposureStore),
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
    return store.size();
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
    // TODO check if we've already entered this source
    store.add(
      ExposureMetadata{
        .agentId = agent, 
        .sensorClassId = sensorClass,
        .sensorInstanceId = instance,
        .modelClassId = ModelT::modelClassId
      }
    );
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
    return store.remove(instanceId);
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
  ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT, ExposureStoreT>>
  analysisDelegate() noexcept {
    return ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT, ExposureStoreT>>(
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
      std::size_t instancePos = store.findMetaBySensorInstanceId(changes[changeIndex].sensorInstanceId);
      if (instancePos < ExposureStoreT::max_size) {
        anyNotified = true;
        auto& contents = store.getContents(instancePos);
        auto exposureIter = contents.begin();
        exposureIter += instancePos; // advance to the position of interest to the receiver
        auto exposureEnd = contents.end();
        handler.exposureLevelChanged(store.getTag(instancePos), exposureIter, exposureEnd); //exposures[instancePos].contents()[0]); // TODO replace this with a safety check
        // TODO fire the above for only those items that have changed, not the whole array or first element, using periodStart and periodEnd in the change metadata
      }
    }
    // reset changes for the next run
    changes.clear();
    return anyNotified;
  }

  /// MARK: Methods invoked by external risk state classes (E.g. Exposure Notification frameworks)

  /**
   * @brief Get the Count of the number of exposure scores recorded for a given instance ID
   * 
   * @param sensorInstanceId The sensor instanceId we are interested in
   * @return std::size_t The number of current, in memory, exposure period values stored
   */
  std::size_t getCountByInstanceId(const UUID& sensorInstanceId) const noexcept {
    std::size_t pos = store.findMetaBySensorInstanceId(sensorInstanceId);
    if (pos >= ExposureStoreT::max_size) {
      // not found - return false
      return 0;
    }
    auto sz = store.getContents(pos).size();
    return sz;
  }

  template <typename ExposureCallableT>
  bool forEachExposure(const UUID& sensorInstanceId, ExposureCallableT callable) const noexcept {
    bool found = false;
    for (std::size_t pos = 0;pos < store.size();++pos) {
      auto& tag = store.getTag(pos);
      auto& siid = tag.sensorInstanceId;
      if (siid == sensorInstanceId) {
        found = true;
        store.over(pos,[&callable, &tag] (const Exposure& score) -> void {
          callable(tag,score);
        });
      }
    }
    return found;
  }

  // TODO convert exposures call, with conditional lambda 
  //      (E.g. to convert particular humanProx exposures to covid19prox exposures)

private:
  CallbackHandlerT& handler;
  ExposureStoreT& store;

  AllocatableArray<ExposureChangeReference,ExposureStoreT::max_size> changes;

  Date anchor;
  TimeInterval period;

  bool running;
  
  void applyAdditionalExposure(const UUID& modelId, const analysis::SampledID sampled, const Date& sampleTaken, const double sampleValue) noexcept {
    std::size_t pos = store.findMetaByModelClassId(modelId);
    if (pos >= ExposureStoreT::max_size) {
      return;
    }
    // Use SampledID as the instanceId for now (might not always be the case depending on source/model)
    auto& exposureArray = store.getContents(pos);
    if (0 == exposureArray.size()) {
      // TODO externalise the API for this into the store class
      exposureArray.add(Exposure{
        .periodStart = sampleTaken,
        .periodEnd = sampleTaken,
        .value = sampleValue, // explicit conversion
        .confidence = 1.0 // TODO pass through analysis API confidence, if supported (compilation option?)
      });
      // Record change for later notification
      recordChange(store.getTag(pos).sensorInstanceId, sampleTaken, sampleTaken);
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
        recordChange(store.getTag(pos).sensorInstanceId, last.periodStart, sampleTaken);
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
        recordChange(store.getTag(pos).sensorInstanceId, exposureArray[0].periodStart, exposureArray[0].periodEnd);
      }
    }
  }

  void recordChange(const UUID& instanceId, const Date& periodStart, const Date& periodEnd) noexcept {
    if (!running) {
      return;
    }
    // Note including start and end dates for efficiency
    // Check for an existing change for this instanceId, and modify its start and end times
    std::size_t cpos = ExposureStoreT::max_size;
    for (std::size_t i = 0;i < changes.size();++i) {
      if (changes[i].sensorInstanceId == instanceId) {
        cpos = i;
      }
    }
    if (ExposureStoreT::max_size != cpos) {
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