//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_EXPOSURE_MANAGER_H
#define HERALD_EXPOSURE_MANAGER_H

#include "agent.h"

#include <array>
#include <functional>

namespace herald {
namespace exposure {

// FWD Declare
template <typename CallbackHandlerT, std::size_t MaxAgents>
class ExposureManager;

namespace {
  struct DefaultNullExposureCallbackHandler {
    void riskLevelChanged(const Agent& notifyAgent,std::size_t notifyLevelId,double notifyCurrentExposureValue) noexcept {
    }
  };
}
using DefaultExposureManager = ExposureManager<DefaultNullExposureCallbackHandler,8>;

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

  ExposureManagerDelegate& operator=(ExposureManagerDelegate& toMove) noexcept {
    manager = toMove.manager;
    return *this;
  }

  ExposureManagerDelegate& operator=(const ExposureManagerDelegate& toCopy) noexcept {
    manager = toCopy.manager;
    return *this;
  }


  ~ExposureManagerDelegate() noexcept = default;

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

#ifndef HERALD_MAX_EXPOSURE_LEVELS_PER_AGENT
#define HERALD_MAX_EXPOSURE_LEVELS_PER_AGENT 8
#endif

// Hidden internal class
namespace {
  struct ExposureLevel {
    double greaterOrEqual = 0.0;
    double lessThan = 100.0;
    std::size_t id = 0;
  };

  struct AgentInfo {
    // Static values
    static constexpr std::size_t MaxLevelsPerAgent = HERALD_MAX_EXPOSURE_LEVELS_PER_AGENT;
    
    // config time values
    Agent agent;
    std::size_t exposureModelClassId = 0;
    std::array<ExposureLevel,HERALD_MAX_EXPOSURE_LEVELS_PER_AGENT> levels;
    std::size_t levelsInUse = 0;
    std::size_t defaultLevelId = 0;
    double initialExposureValue = 0.0;

    // Runtime values
    double currentExposureValue = 0.0;
    std::size_t currentExposureLevelId = 0;
    std::size_t priorCheckExposureLevelId = 0; // used to flag changes to listeners, once
  };
} // end anonymous namespace


template <typename CallbackHandlerT, std::size_t MaxAgents>
class ExposureManager {
public:
  static constexpr std::size_t MaxSize = MaxAgents;
  // using delegate_type = std::remove_cv_t<
  //   ExposureManagerDelegate<ExposureManager<CallbackHandlerT,MaxAgents>>
  // >;

  ExposureManager(CallbackHandlerT& callbackHandler) noexcept
   : agents(),
     count(0),
     handler(callbackHandler),
     running(false)
  {
    ;
  }

  ~ExposureManager() noexcept = default;

  const bool isRunning() const noexcept {
    return running;
  }

  void setRunning(bool newStatus) noexcept {
    running = newStatus;
  }

  const std::size_t agentCount() const noexcept {
    return count;
  }

  bool addAgent(const Agent& toCopy) noexcept {
    if (count >= MaxSize) {
      return false;
    }
    agents[count] = AgentInfo{.agent=toCopy};
    ++count;
    return true;
  }

  bool removeAgent(const Agent& toRemove) noexcept {
    std::size_t pos = findAgent(toRemove);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return false;
    }
    for (std::size_t mp = pos;mp < count - 1;++mp) {
      agents[mp] = agents[mp + 1];
    }
    --count;
    return true;
  }

  template <typename ExposureScoreT>
  ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT,MaxAgents>>
  analysisDelegate() noexcept {
    return ExposureManagerDelegate<ExposureScoreT,ExposureManager<CallbackHandlerT,MaxAgents>>(
      *this
    );
  }

  template <typename ModelT>
  bool setAgentExposureModel(const Agent& agent) noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return false;
    }
    agents[pos].exposureModelClassId = ModelT::output_value_type::classId;
    return true;
  }

  bool addExposureLevel(const Agent& agent,double greaterOrEqual,double lessThan, std::size_t levelId) noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return false;
    }
    AgentInfo& ai = agents[pos];
    if (ai.levelsInUse >= AgentInfo::MaxLevelsPerAgent) {
      return false;
    }
    ai.levels[ai.levelsInUse] = ExposureLevel{
      .greaterOrEqual = greaterOrEqual,
      .lessThan = lessThan,
      .id = levelId
    };
    ++ai.levelsInUse;
    return true;
  }

  std::size_t getExposureLevelCount(const Agent& agent) const noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return 0;
    }
    return agents[pos].levelsInUse;
  }

  bool setExposureDefaults(const Agent& agent,double initialValue,std::size_t levelId) noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return false;
    }
    AgentInfo& ai = agents[pos];
    ai.initialExposureValue = initialValue;
    ai.currentExposureValue = initialValue;
    ai.defaultLevelId = levelId;
    ai.currentExposureLevelId = levelId;
    return true;
  }

  std::size_t getExposureDefaultLevelId(const Agent& agent) const noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return 0;
    }
    return agents[pos].defaultLevelId;
  }

  double getExposureInitialValue(const Agent& agent) const noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return 0;
    }
    return agents[pos].initialExposureValue;
  }

  double getExposureCurrentValue(const Agent& agent) const noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return 0;
    }
    return agents[pos].currentExposureValue;
  }

  std::size_t getExposureCurrentLevelId(const Agent& agent) const noexcept {
    std::size_t pos = findAgent(agent);
    if (pos >= MaxAgents) {
      // agent not found - return false
      return 0;
    }
    return agents[pos].currentExposureLevelId;
  }

  template <typename ValT>
  bool applyAdditionalExposure(herald::analysis::SampledID sampled,herald::analysis::Sample<ValT> sample) noexcept {
    // Assume any Risk value type in Herald has classId and doubleValue() members
    // TODO add template warning message about unviable types to aid custom type developers
    // Note: We ignore the source (SampleID) because we aggregated exposure no matter the source by default
    std::size_t sampleClassId = ValT::classId;
    double incrementalValue = (double)sample;
    bool found = false;
    for (std::size_t pos = 0;pos < count;++pos) {
      if (agents[pos].exposureModelClassId == sampleClassId) {
        agents[pos].currentExposureValue += incrementalValue;
        found = true;
      }
    }
    return found;
  }

  void evaluateLevels() noexcept {
    // for all agents
    for (std::size_t pos = 0;pos < count;++pos) {
      AgentInfo& ai = agents[pos];
      // evaluate all levels' bounds against current exposure score
      for (std::size_t levelIdx = 0; levelIdx < ai.levelsInUse;++levelIdx) {
        // set new level to appropriate one
        if (ai.currentExposureValue >= ai.levels[levelIdx].greaterOrEqual &&
            ai.currentExposureValue < ai.levels[levelIdx].lessThan)
        {
          ai.currentExposureLevelId = ai.levels[levelIdx].id;
        }
      }
      // if prior level is not same as new level, raise an event to the handler
      if (ai.priorCheckExposureLevelId != ai.currentExposureLevelId) {
        handler.riskLevelChanged(ai.agent,ai.currentExposureLevelId,ai.currentExposureValue);
        // set new level as prior level (so we don't call the handler again)
        ai.priorCheckExposureLevelId = ai.currentExposureLevelId;
      }
    }
  }

private:
  std::array<AgentInfo,MaxAgents> agents;
  std::size_t count;
  CallbackHandlerT& handler;
  bool running;

  std::size_t findAgent(const Agent& agent) const noexcept {
    for (std::size_t pos = 0;pos < count;++pos) {
      if (agents[pos].agent.id == agent.id) {
        return pos;
      }
    }
    return MaxAgents;
  }
};

}
}

#endif