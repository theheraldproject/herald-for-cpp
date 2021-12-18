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

using namespace herald::datatype;

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
     scores(),
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
    std::size_t pos = findMetaByModelInstanceId(riskModelInstanceId);
    if (max_size == pos) {
      // Instance does not yet exist
      // If not, ensure we have space for a new one
      if (max_size == scores.size()) {
        return false;
      }
      // Add data for the new one
      // Also add dirty flag (non dirty by default) for this period too
      bool added = scores.add(RiskScoreArray<max_size>{
        RiskScoreMetadata{
          .agentId = agent, // RISK model agent, NOT exposure source agent(s)
          .algorithmId = riskModelInstanceId
        }
      }); // New instance, with defaults
      if (!added) {
        return false;
      }
      instanceMetadata[scores.size() - 1] = RiskModelInstanceMetadata{}; // defaults (not dirty)
    } else {
      // Instance already exists
      // update meta only
      // None at the moment...
    }
    return true;
  }

  /// MARK: Event driven member functions

  // RiskManagerExposureCallbackHandler& getExposureCallbackHandler() noexcept
  // {
  //   return exposureCallbackHandler;
  // }

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
      for (std::size_t pos = 0;pos < scores.size();++pos) {
        // Find linked model by algorithmId
        const UUID& algorithmId = scores[pos].getTag().algorithmId;
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
    for (std::size_t p = 0;p < scores.size();++p) {
      // Find linked model by algorithmId
      const UUID& algorithmId = scores[p].getTag().algorithmId;
      bool ok = true;
      auto& instanceMetadataValue = instanceMetadata[p];
      Date startTime{0};
      Date endTime{0};
      calculateOverlappingTimePeriod(
        instanceMetadataValue.periodStart, instanceMetadataValue.periodEnd,
        scores[p].contents(),
        startTime, endTime
      );
      // TODO consider adding an if for startTime != endTime to guard the below if there's no data (minor perf enhancement)
      models.forMatchingAlgorithm([this, &src, &startTime, &endTime, &ok] (auto&& algo) {
        ok = ok & algo.produce(
          parameters,
          src, // TODO add aggregate call to ExposureManager
          startTime,
          endTime,
          period, // TODO make this per risk score type, not global
          store
        );
      }, algorithmId);
      instanceMetadataValue.dirty = false;
    }
  }

  /// MARK : Risk Score value management methods


  std::size_t getRiskScoreCount(const UUID& riskModelInstanceId) const noexcept
  {
    std::size_t pos = findMetaByModelInstanceId(riskModelInstanceId);
    if (pos >= max_size) {
      return 0;
    }
    auto sz = scores[pos].contents().size();
    return sz;
  }

  /**
   * @brief Executes a risk score callable for each risk score, in ascending date order, if any exist.
   * 
   * @param riskModelId The model instance to return risk scores for
   * @return true If a risk score exists for this model instance and the callable was called at least once
   * @return false If no risk score exists for this model instance
   */
  template <typename RiskScoreCallableT>
  bool forEachRiskScore(const UUID& riskModelInstanceId, RiskScoreCallableT callable) const noexcept
  {
    // TODO fill out this method
    return false;
  }

private:
  RiskModelsT models;
  RiskParametersT parameters;
  RiskScoreStoreT& store;

  Date anchor;
  TimeInterval period;

  /// \brief In memory ephemeral cached RiskScores
  RiskScoreSet<max_size> scores;
  std::array<RiskModelInstanceMetadata,max_size> instanceMetadata;

  std::size_t inline findMeta(const RiskScoreMetadata& meta) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      if (scores[pos].getTag() == meta) {
        return pos;
      }
    }
    return max_size;
  }

  std::size_t inline findMetaByAgentId(const UUID& agentId) const noexcept {
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

  std::size_t inline findMetaByModelInstanceId(const UUID& riskModelInstanceId) const noexcept {
    for (std::size_t pos = 0;pos < scores.size();++pos) {
      auto& mcid = scores[pos].getTag().algorithmId;
      if (mcid == riskModelInstanceId) {
        return pos;
      }
    }
    return max_size;
  }
  
  template <typename RiskScoreArrayT>
  void calculateOverlappingTimePeriod(
    const Date& periodStart, const Date& periodEnd,
    const RiskScoreArrayT& riskScores,
    Date& outputStartTime, Date& outputEndTime)
  {
    const std::size_t sz = riskScores.size();
    if (0 == sz) {
      return;
    }
    outputStartTime = riskScores[0].periodStart;
    outputEndTime = riskScores[0].periodEnd;
    for (std::size_t idx = 1;idx < sz;++idx) {
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