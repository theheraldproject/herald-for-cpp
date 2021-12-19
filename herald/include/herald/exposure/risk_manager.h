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
#include <utility>

namespace herald {
namespace exposure {

using namespace herald::datatype;


/**
 * @brief A basic in-memory store of Risk Score information
 * 
 * This class is intended to be used standalone, and does not contain memory cache logic.
 * 
 * @tparam MaxInMemoryRiskSummaries The maximum number of Risk Score arrays to maintain in RAM.
 */
template <std::size_t MaxInMemoryRiskSummaries>
class FixedMemoryRiskStore {
public:
  /**
   * @brief The maximum number of exposure sources this exposure store supports
   * 
   */
  static constexpr std::size_t max_size = MaxInMemoryRiskSummaries;

  /**
   * @brief Construct a new Fixed Memory Exposure Store object.
   * Default constructor
   */
  FixedMemoryRiskStore()
   : scores()
  {
    ;
  }

  /// MARK: Event driven scoring methods to add a score to our list:-
  void score(const RiskScoreMetadata& addTo, herald::datatype::RiskScore&& toStore) noexcept {
    std::size_t pos = findMeta(addTo);
    if (max_size == pos) {
      return;
    }
    // otherwise, add the score
    scores[pos].contents().add(toStore);
    // TODO ensure that if it already exists for the time period, we replace and don't add
  }

  /// MARK: Exposure array and elements access and size methods

  /**
   * @brief Provisions space, if available, for the given ExposureMetadata
   * 
   * @param meta The ExposureMetadata to provision storage for
   * @return true If the metadata already has storage provisioned, or if provisioning of new storage succeeded
   * @return false If storage could not be provisioned for this exposure metadata
   */
  bool add(RiskScoreMetadata meta) noexcept {
    // TODO check if we already have provisioned storage for this metadata description
    if (scores.size() >= max_size) {
      return false;
    }
    return scores.add(
      RiskScoreArray<max_size>{
        meta
      }
    );
  }

  // TODO add methods for adding/removing individual risk score values to the array created in add(), above

  /**
   * @brief Removes the whole set of risk score information for the given RiskScoreMetadata::instanceId value
   * 
   * @param instanceId The Instance ID (UUID) of the RiskScoreMetadata to remove from storage.
   * @return true If the instanceId was found (and thus removed)
   * @return false If the instanceId was not found
   */
  bool remove(const UUID& instanceId) noexcept {
    std::size_t pos = findMetaBySensorInstanceId(instanceId);
    if (pos >= max_size) {
      // agent not found - return false
      return false;
    }
    auto count = scores.size();
    for (std::size_t mp = pos;mp < count - 1;++mp) {
      scores[mp] = scores[mp + 1];
    }
    return true;
  }

  /**
   * @brief Returns the number of currently allocated RiskScore arrays
   * 
   * @return const std::size_t The number of arrays currently used. Always <= max_size.
   */
  const std::size_t size() const noexcept {
    return scores.size();
  }

  /**
   * @brief Get the Contents for the given Risk Score position
   * 
   * WARNING: does not validate the pos value passed to it yet.
   * 
   * @param pos Position of the allocated Risk Score metadata to return the RiskScoreArray for
   * @return auto& The contents Array (RiskScoreArray instance)
   */
  auto& getContents(std::size_t pos) noexcept {
    return scores[pos].contents();
  }

  /**
   * @brief Get the Tag object for the RiskScoreMetadata instance at the given pos position
   * 
   * WARNING: does not validate the pos value passed to it yet.
   * 
   * @param pos Position of the allocated risk score metadata to return the tag for
   * @return auto& The Tag (RiskScoreMetadata instance) at the given position
   */
  auto& getTag(std::size_t pos) noexcept {
    return scores[pos].getTag();
  }

  template <typename CallableT>
  void over(std::size_t pos, CallableT callable) noexcept {
    auto iter = scores[pos].ccontents().cbegin();
    auto end = scores[pos].ccontents().cend();
    for (;iter != end;++iter) {
      const auto& score = *iter;
      callable(score);
    }
  }

  std::size_t findMeta(const RiskScoreMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      if (scores[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaByAgentId(const Agent& agentId) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      auto& sc = scores[pos];
      auto& t = sc.getTag();
      auto& siid = t.agentId;
      if (siid == agentId) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaByAlgorithmId(const AlgorithmId& algoId) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      auto& mcid = scores[pos].getTag().algorithmId;
      if (mcid == algoId) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t findMetaByModelInstanceId(const UUID& riskModelInstanceId) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      auto& mcid = scores[pos].getTag().instanceId;
      if (mcid == riskModelInstanceId) {
        return pos;
      }
    }
    return max_size;
  }

private:
  /// \brief In memory ephemeral cached risk scores
  RiskScoreSet<max_size> scores;
};

// 'hidden' namespace
namespace {

/**
 * @brief Provides a convenient runtime wrapper around any underlying RiskScoreStore 
 * for linking new scores to risk score metadata without revealing the data to the algorithm 
 * instance generting it
 * 
 * @tparam RiskScoreT The underlying RiskStore to wrap
 */
template <typename RiskScoreT>
class WrappedRiskScoreStore {
public:
  WrappedRiskScoreStore(RiskScoreT& riskStore, const RiskScoreMetadata& linkTo) noexcept
   : store(riskStore), linked(linkTo)
  {
    ;
  }

  ~WrappedRiskScoreStore() noexcept = default;

  void score(herald::datatype::RiskScore&& toStore) noexcept {
    store.score(linked,std::forward<herald::datatype::RiskScore>(toStore));
  }

private:
  RiskScoreT& store;
  const RiskScoreMetadata& linked;
};

} // end 'hidden' namespace


// FWD DECL
template <typename RiskModelsT, typename RiskParametersT,
          std::size_t MaxInMemoryRiskScoreSummaries, typename RiskScoreStoreT>
class RiskManager;

template <typename RiskManagerT, typename ExposureStoreT>
class RiskManagerExposureCallbackAdapter {
public:
  explicit RiskManagerExposureCallbackAdapter(RiskManagerT& managerRef, ExposureStoreT& exposureStoreRef) noexcept
   : riskManager(managerRef), exposureStore(exposureStoreRef)
  {
    ;
  }

  ~RiskManagerExposureCallbackAdapter() = default;

  template <typename IterT>
  void exposureLevelChanged(
    const herald::datatype::ExposureMetadata& meta,
    IterT& iter,
    IterT& end) noexcept
  {
    riskManager.injectExposureChanges(exposureStore,meta,iter,end);
  }

private:
  RiskManagerT& riskManager;
  ExposureStoreT& exposureStore;
};

namespace {
struct RiskModelInstanceMetadata {
  // Standard metadata

  // Dynamic runtime callback metadata
  bool dirty = false;
  Date periodStart = Date{0};
  Date periodEnd = Date{0};
};
}

// FWD DECL
// template <typename RiskModelsT, typename RiskParametersT,
//           std::size_t MaxInMemoryRiskScoreSummaries, typename RiskScoreStoreT>
// class RiskManager;
// Full definition
template <typename RiskModelsT, typename RiskParametersT,
          std::size_t MaxInMemoryRiskScoreSummaries, typename RiskScoreStoreT>
class RiskManager {
public:
  static constexpr std::size_t max_size = MaxInMemoryRiskScoreSummaries;

  RiskManager(RiskModelsT&& riskModelsToOwn, RiskParametersT&& riskParametersToOwn, RiskScoreStoreT& initialRiskScoreStore)
   : models(std::move(riskModelsToOwn)),
     parameters(std::move(riskParametersToOwn)),
     store(initialRiskScoreStore),
     anchor(Date{}), // Default to instantiation DateTime
     period(TimeInterval::hours(24)), // Default to one day interval
    //  scores(),
     instanceMetadata()
  {
    // static asserts (if applicable)
    ;
  }

  ~RiskManager() = default;

  /// MARK: Risk Manager Configuration

  void setGlobalPeriodInterval(const Date recentIndexDateTime, const TimeInterval interval) noexcept
  {
    anchor = recentIndexDateTime; // Do we handle anchor time that is in the future? No - could be a future simulation
    period = interval;
  }

  const Date getGlobalPeriodAnchor() const noexcept {
    return anchor;
  }

  const TimeInterval getGlobalPeriodInterval() const noexcept {
    return period;
  }

  template <typename RiskModelT>
  bool addRiskModel(const UUID& riskModelInstanceId, const Agent& agent, const RiskModelT& modelRef) noexcept
  {
    // If risk model config exists, set static model config
    std::size_t pos = store.findMetaByModelInstanceId(riskModelInstanceId);
    if (max_size == pos) {
      // Instance does not yet exist
      // If not, ensure we have space for a new one
      if (max_size == store.size()) {
        return false;
      }
      // Add data for the new one
      // Also add dirty flag (non dirty by default) for this period too
      bool added = store.add(
        RiskScoreMetadata{
          .agentId = agent, // RISK model agent, NOT exposure source agent(s)
          .algorithmId = modelRef.algorithmId,
          .instanceId = riskModelInstanceId
        }
      ); // New instance, with defaults
      if (!added) {
        return false;
      }
      instanceMetadata[store.size() - 1] = RiskModelInstanceMetadata{}; // defaults (not dirty)
    } else {
      // Instance already exists
      // update meta only
      // None at the moment...
    }
    return true;
  }

  /// MARK: Event driven member functions

  template <typename ExposureSrcT, typename IterT>
  void injectExposureChanges(
    const ExposureSrcT& src,
    const herald::datatype::ExposureMetadata& meta,
    IterT& iter,
    IterT& end) noexcept
  {
    for (; iter != end;++iter) {
      auto& exposure = *iter;
      // Find any matching algorithms for this exposure source
      for (std::size_t pos = 0;pos < store.size();++pos) {
        // Find linked model by algorithmId
        const AlgorithmId& algorithmId = store.getTag(pos).algorithmId;
        models.forMatchingAlgorithm([this, &pos, &meta, &exposure] (auto&& algo) {
          if (algo.potentiallyDirty(meta.agentId, exposure)) {
            // Mark each as dirty for this time period
            if (instanceMetadata[pos].dirty) {
              instanceMetadata[pos].periodStart = instanceMetadata[pos].periodStart < exposure.periodStart ? instanceMetadata[pos].periodStart : exposure.periodStart;
              instanceMetadata[pos].periodEnd = instanceMetadata[pos].periodEnd > exposure.periodEnd ? instanceMetadata[pos].periodEnd : exposure.periodEnd;
            } else {
              instanceMetadata[pos].dirty = true;
              instanceMetadata[pos].periodStart = exposure.periodStart;
              instanceMetadata[pos].periodEnd = exposure.periodEnd;
            }
          }
        }, algorithmId);
      }
    }
    // Now run through the relevant time period values, replacing the previous with the current values (or adding new ones)
    // Also Reset dirty flags
    for (std::size_t p = 0;p < store.size();++p) {
      // Find linked model by algorithmId
      const auto riskScoreMeta = store.getTag(p);
      const AlgorithmId& algorithmId = riskScoreMeta.algorithmId;
      bool ok = true;
      auto& instanceMetadataValue = instanceMetadata[p];
      Date startTime{0};
      Date endTime{0};
      calculateOverlappingTimePeriod(
        instanceMetadataValue.periodStart, instanceMetadataValue.periodEnd,
        store.getContents(p),
        startTime, endTime
      );
      // TODO consider adding an if for startTime != endTime to guard the below if there's no data (minor perf enhancement)
      WrappedRiskScoreStore rss{store,riskScoreMeta};
      models.forMatchingAlgorithm([this, &src, &startTime, &endTime, &ok, &rss] (auto&& algo) {
        ok = ok & algo.produce(
          parameters,
          src,
          startTime,
          endTime,
          period, // TODO make this per risk score type, not global
          rss // injects RiskScoreMetadata before invoking the underlying RiskStore
        );
      }, algorithmId);
      instanceMetadataValue.dirty = false;
    }
  }

  /// MARK : Risk Score value management methods


  std::size_t getRiskScoreCount(const UUID& riskModelInstanceId) const noexcept
  {
    std::size_t pos = store.findMetaByModelInstanceId(riskModelInstanceId);
    if (pos >= max_size) {
      return 0;
    }
    auto sz = store.getContents(pos).size();
    return sz;
  }

  /**
   * @brief Executes a risk score callable for each risk score, in ascending date order, if any exist.
   * 
   * @param riskModelInstanceId The model instance to return risk scores for
   * @param callable A function or lambda with the signature (const RiskScoreMetadata& meta, const RiskScore& score) -> bool
   * @return true If a risk score exists for this model instance and the callable was called at least once
   * @return false If no risk score exists for this model instance
   */
  template <typename RiskScoreCallableT>
  bool forEachRiskScore(const UUID& riskModelInstanceId, RiskScoreCallableT callable) const noexcept
  {
    bool found = false;
    for (std::size_t pos = 0;pos < store.size();++pos) {
      auto& tag = store.getTag(pos);
      auto& mcid = tag.instanceId;
      if (mcid == riskModelInstanceId) {
        found = true;
        store.over(pos,[&callable, &tag] (const RiskScore& score) -> void {
          callable(tag,score);
        });
      }
    }
    return found;
  }

private:
  RiskModelsT models;
  RiskParametersT parameters;
  RiskScoreStoreT& store;

  Date anchor;
  TimeInterval period;

  /// \brief In memory ephemeral cached RiskScores
  // RiskScoreSet<max_size> scores;
  std::array<RiskModelInstanceMetadata,max_size> instanceMetadata;
  
  template <typename RiskScoreArrayT>
  void calculateOverlappingTimePeriod(
    const Date& periodStart, const Date& periodEnd,
    const RiskScoreArrayT& riskScores,
    Date& outputStartTime, Date& outputEndTime)
  {
    // MUST set the overlapping time period to the input time period, to ensure generation the FIRST time!
    outputStartTime = periodStart;
    outputEndTime = periodEnd;
    const std::size_t sz = riskScores.size();
    // if (0 == sz) {
    //   return;
    // }
    // outputStartTime = riskScores[0].periodStart;
    // outputEndTime = riskScores[0].periodEnd;
    for (std::size_t idx = 0;idx < sz;++idx) {
      if (outputStartTime > riskScores[idx].periodStart) {
        outputStartTime = riskScores[idx].periodStart;
      }
      if (outputEndTime < riskScores[idx].periodEnd) {
        outputEndTime = riskScores[idx].periodEnd;
      }
    }
  }
};

// Deduction guides
template <typename RiskModelsT, typename RiskParametersT, std::size_t MaxSize = 8, typename RiskScoreStoreT>
explicit RiskManager(RiskModelsT&&,RiskParametersT&&,RiskScoreStoreT&) -> 
  RiskManager<RiskModelsT,RiskParametersT,MaxSize,RiskScoreStoreT>;



}
}

#endif