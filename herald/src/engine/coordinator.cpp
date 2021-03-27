//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/context.h"
#include "herald/engine/activities.h"
#include "herald/engine/coordinator.h"
#include "herald/data/sensor_logger.h"

#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <optional>
// #include <utility>
// #include <future>

namespace herald {
namespace engine {

using namespace herald::datatype;

template <typename ContextT>
class Coordinator<ContextT>::Impl {
public:
  Impl(ContextT& ctx);
  ~Impl();

  ContextT& context;

  std::vector<std::shared_ptr<CoordinationProvider>> providers;
  std::map<FeatureTag,std::shared_ptr<CoordinationProvider>> featureProviders;

  bool running;

  HLOGGER(ContextT);
};

template <typename ContextT>
Coordinator<ContextT>::Impl::Impl(ContextT& ctx)
  : context(ctx),
    providers(),
    running(false)
    HLOGGERINIT(ctx,"engine","coordinator")
{
  ;
}

template <typename ContextT>
Coordinator<ContextT>::Impl::~Impl()
{
  ;
}




template <typename ContextT>
Coordinator<ContextT>::Coordinator(ContextT& ctx)
  : mImpl(std::make_unique<Impl>(ctx))
{
  ;
}

template <typename ContextT>
Coordinator<ContextT>::~Coordinator()
{
  ;
}


/** Introspect and include in iteration planning **/
template <typename ContextT>
void
Coordinator<ContextT>::add(std::shared_ptr<Sensor> sensor)
{
  HDBG("Adding sensor");
  auto prov = sensor->coordinationProvider();
  if (prov.has_value()) {
    HDBG("Sensor has Provider implementation");
    mImpl->providers.push_back(prov.value());
  }
}

/** Remove from iteration planning **/
template <typename ContextT>
void
Coordinator<ContextT>::remove(std::shared_ptr<Sensor> sensor)
{
  // TODO support remove
}

/** Prepares for iterations to be called (may pre-emptively make calls) **/
template <typename ContextT>
void
Coordinator<ContextT>::start()
{
  HDBG("Start called");
  // Clear feature providers
  mImpl->featureProviders.clear();
  // Fetch feature providers
  for (auto prov: mImpl->providers) {
    auto myFeatures = prov->connectionsProvided();
    for (auto feature : myFeatures) {
      mImpl->featureProviders.emplace(feature,prov);
    }
  }
  mImpl->running = true;
  HDBG("Start returning");
}

/** Execute an iteration of activity, according to settings **/
template <typename ContextT>
void
Coordinator<ContextT>::iteration()
{
  if (!mImpl->running) {
    HDBG("Coordinator not running. Returning from iteration having done nothing.");
    return;
  }
  HDBG("################# ITERATION #################");
  // HDBG("Entered iteration");
  // Create empty list of required prereqs per provider
  std::map<std::shared_ptr<CoordinationProvider>,std::vector<PrioritisedPrerequisite>> assignPrereqs;
  for (auto& prov : mImpl->providers) {
    assignPrereqs.emplace(prov,std::vector<PrioritisedPrerequisite>());
  }
  // HDBG("Completed initialisation of provider prerequisities containers");
  // HDBG(" - Provider count: {}", mImpl->providers.size());
  
  std::vector<PrioritisedPrerequisite> connsRequired;
  // Loop over providers and ask for feature pre-requisites
  for (auto& prov : mImpl->providers) {
    auto myConns = prov->requiredConnections();
    std::copy(myConns.begin(),myConns.end(),
      std::back_insert_iterator<std::vector<PrioritisedPrerequisite>>(connsRequired));
  }
  // HDBG(std::to_string(connsRequired.size()));
  // HDBG("Retrieved providers' current prerequisites");
  // TODO de-duplicate pre-reqs
  // Now link required prereqs to each provider
  for (auto& p : connsRequired) {
    auto el = mImpl->featureProviders.find(std::get<0>(p)); // find provider for given prereq by feature tag
    if (mImpl->featureProviders.end() != el) {
      assignPrereqs[el->second].push_back(p);
    }
  }
  // HDBG("Linked pre-reqs to their providers");

  // Some debug checks here
  int cnt = 0;
  for (auto& ass : assignPrereqs) {
    // HDBG("assign prereqs number {} has this many prereqs to fill {}", cnt, ass.second.size());
    cnt++;
  }
  
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
    std::vector<PrioritisedPrerequisite> myProvisioned = prov.first->provision(prov.second);
    std::copy(myProvisioned.begin(),myProvisioned.end(),
      std::back_insert_iterator<std::vector<PrioritisedPrerequisite>>(provisioned));
  }
  // HDBG("All pre-requisities requests sent and responses received");
  // TODO do the above asynchronously and await callback or timeout for all

  // For each which are now present, ask for activities (in descending priority order)
  for (auto& prov : mImpl->providers) {
    auto maxActs = prov->requiredActivities();
    // TODO sort by descending priority before actioning
    for (auto& act : maxActs) {
      std::string san("Activity ");
      san += act.name;
      HDBG(san);
      // HDBG("Checking next desired activity for prereqs being satisfied");
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
          HDBG(" - Prereq satisfied");
        } else {
          HDBG(" - Prereq NOT SATISFIED");
        }
      }
      // Carry out activities with completion callbacks passed in
      if (allFound) {
        HDBG("All satisfied, calling activity");
        // do activity

        // FOR PLATFORMS WITH STD::ASYNC
        // act.executor(act,[this] (Activity act, std::optional<Activity> followOn) -> void {
        //   // TODO handle result
        //   // TODO Carry out any follow up activities
        //   HDBG("Activity completion callback called");
        // });

        // FOR PLATFORMS WITHOUT
        std::optional<Activity> followOn = act.executor(act);
        // TODO carry out follow on activity until no more follow ons (or max follow on number hit)
      }
    }
  }
  // HDBG("Leaving iteration");
  HDBG("#################    END    #################");
}

/** Closes out any existing connections/activities **/
template <typename ContextT>
void
Coordinator<ContextT>::stop()
{
  mImpl->running = false;
  // No-op - done in other methods (for now)
}

bool operator<(const std::shared_ptr<CoordinationProvider>& first, const std::shared_ptr<CoordinationProvider>& second)
{
  return &(*first) < &(*second); // simple memory address comparator of item pointed TO
}

}
}