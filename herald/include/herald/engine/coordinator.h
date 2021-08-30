//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_COORDINATOR_H
#define HERALD_COORDINATOR_H

#include "../context.h"
#include "activities.h"
#include "../data/sensor_logger.h"

#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <optional>

namespace herald {

/// \brief Engine classes provide for task scheduling, including complex inter-dependent tasks.
namespace engine {

///
/// \brief Coordinates all connection and activities used across all sensors within Herald
/// 
/// Responsible for:-
///
/// - Determining Sensor capabilities and requirements around connections and Activity instances
/// 
/// Manages:- 
///
/// - Nothing, but coordinates activities throughout Herald Sensor networks on behalf of SensorArray
/// 
/// Is managed by:-
///
/// - SensorArray
///
template <typename ContextT>
class Coordinator {
public:
  /// Default constructor. Receives a configured platform-specific context instance.
  Coordinator(ContextT& ctx)
  : context(ctx),
    providers(),
    running(false)
    HLOGGERINIT(ctx,"engine","coordinator")
  {}

  ~Coordinator() = default;

  /// Introspect and include in iteration planning
  template <typename SensorT>
  void add(SensorT& sensor) {
    HTDBG("Adding sensor");
    auto prov = sensor.coordinationProvider();
    if (prov.has_value()) {
      HTDBG("Sensor has Provider implementation");
      providers.push_back(prov.value());
    }
  }
  /// Remove from iteration planning
  template <typename SensorT>
  void remove(SensorT& sensor)
  {
    // TODO support remove
  }

  /// Prepares for iterations to be called (may pre-emptively make calls)
  void start() {
    HTDBG("Start called");
    // Clear feature providers
    featureProviders.clear();
    // Fetch feature providers
    for (auto prov: providers) {
      auto myFeatures = prov.get().connectionsProvided();
      for (auto feature : myFeatures) {
        featureProviders.emplace(feature,prov);
      }
    }
    running = true;
    HTDBG("Start returning");
  }

  /// Execute an iteration of activity, according to settings
  void iteration() {
    if (!running) {
      HTDBG("Coordinator not running. Returning from iteration having done nothing.");
      return;
    }
    HTDBG("################# ITERATION #################");
    // HTDBG("Entered iteration");
    // Create empty list of required prereqs per provider
    std::map<std::reference_wrapper<CoordinationProvider>,std::vector<PrioritisedPrerequisite>> assignPrereqs;
    for (auto& prov : providers) {
      assignPrereqs.emplace(prov,std::vector<PrioritisedPrerequisite>());
    }
    // HTDBG("Completed initialisation of provider prerequisities containers");
    // HTDBG(" - Provider count: {}", providers.size());
    
    std::vector<PrioritisedPrerequisite> connsRequired;
    // Loop over providers and ask for feature pre-requisites
    for (auto& prov : providers) {
      auto myConns = prov.get().requiredConnections();
      std::copy(myConns.begin(),myConns.end(),
        std::back_insert_iterator<std::vector<PrioritisedPrerequisite>>(connsRequired));
    }
    // HTDBG(std::to_string(connsRequired.size()));
    // HTDBG("Retrieved providers' current prerequisites");
    // TODO de-duplicate pre-reqs
    // Now link required prereqs to each provider
    for (auto& p : connsRequired) {
      auto el = featureProviders.find(std::get<0>(p)); // find provider for given prereq by feature tag
      if (featureProviders.end() != el) {
        assignPrereqs[el->second].push_back(p);
      }
    }
    // HTDBG("Linked pre-reqs to their providers");

    // // Some debug checks here
    // int cnt = 0;
    // for (auto& ass : assignPrereqs) {
    //   // HTDBG("assign prereqs number {} has this many prereqs to fill {}", cnt, ass.second.size());
    //   cnt++;
    // }
    
    // Communicate with relevant feature providers and request features for targets (in descending priority order)
    //  - Includes removal of previous features no longer needed
    std::vector<PrioritisedPrerequisite> provisioned;
    for (auto& prov : assignPrereqs) {
      // TODO sort by descending priority before passing on

      // FOR PLATFORMS WITH STD::FUTURE AND STD::ASYNC
      // std::future<void> fut = std::async(std::launch::async,
      //     &CoordinationProvider::provision, prov.first,
      // //prov.first->provision(
      //     prov.second,[&provisioned] (
      //   const std::vector<PrioritisedPrerequisite> myProvisioned) -> void {
      //   std::copy(myProvisioned.begin(),myProvisioned.end(),
      //     std::back_insert_iterator<std::vector<PrioritisedPrerequisite>>(provisioned));
      // });
      // fut.get(); // waits for callback // TODO wait with timeout

      // FOR OTHER PLATFORMS (E.g. ZEPHYR):-
      std::vector<PrioritisedPrerequisite> myProvisioned = prov.first.get().provision(prov.second);
      std::copy(myProvisioned.begin(),myProvisioned.end(),
        std::back_insert_iterator<std::vector<PrioritisedPrerequisite>>(provisioned));
    }
    // HTDBG("All pre-requisities requests sent and responses received");
    // TODO do the above asynchronously and await callback or timeout for all

    // For each which are now present, ask for activities (in descending priority order)
    for (auto& prov : providers) {
      auto maxActs = prov.get().requiredActivities();
      // TODO sort by descending priority before actioning
      for (auto& act : maxActs) {
        std::string san("Activity ");
        san += act.name;
        HTDBG(san);
        // HTDBG("Checking next desired activity for prereqs being satisfied");
        // Filter requested by provisioned
        bool allFound = true;
        for (auto& pre : act.prerequisites) {
          bool myFound = false;
          for (auto& exists : provisioned) {
            if (std::get<0>(pre) == std::get<0>(exists) &&
                std::get<1>(pre) == std::get<2>(exists)) {
              myFound = true;
            }
          }
          allFound = allFound & myFound;
          if (myFound) {
            HTDBG(" - Prereq satisfied");
          } else {
            HTDBG(" - Prereq NOT SATISFIED");
          }
        }
        // Carry out activities with completion callbacks passed in
        if (allFound) {
          HTDBG("All satisfied, calling activity");
          // do activity

          // FOR PLATFORMS WITH STD::ASYNC
          // act.executor(act,[this] (Activity act, std::optional<Activity> followOn) -> void {
          //   // TODO handle result
          //   // TODO Carry out any follow up activities
          //   HTDBG("Activity completion callback called");
          // });

          // FOR PLATFORMS WITHOUT
          std::optional<Activity> followOn = act.executor(act);
          // TODO carry out follow on activity until no more follow ons (or max follow on number hit)
        }
      }
    }
    // HTDBG("Leaving iteration");
    HTDBG("#################    END    #################");
  }
  /// Closes out any existing connections/activities
  void stop() {
    running = false;
  }

private:
  ContextT& context;

  std::vector<std::reference_wrapper<CoordinationProvider>> providers;
  std::map<FeatureTag,std::reference_wrapper<CoordinationProvider>> featureProviders;

  bool running;

  HLOGGER(ContextT);
};

// /** Comparator for less than (use in maps) **/
bool operator<(const std::reference_wrapper<CoordinationProvider> first, const std::reference_wrapper<CoordinationProvider> second);

}
}

#endif