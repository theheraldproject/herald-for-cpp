//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "../context.h"
#include "../sensor.h"

#include <memory>

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
  Coordinator(ContextT& context);
  ~Coordinator();

  /// Introspect and include in iteration planning
  void add(std::shared_ptr<Sensor> sensor);
  /// Remove from iteration planning
  void remove(std::shared_ptr<Sensor> sensor);

  /// Prepares for iterations to be called (may pre-emptively make calls)
  void start();
  /// Execute an iteration of activity, according to settings
  void iteration();
  /// Closes out any existing connections/activities
  void stop();

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

/** Comparator for less than (use in maps) **/
bool operator<(const std::shared_ptr<CoordinationProvider>& first, const std::shared_ptr<CoordinationProvider>& second);

}
}

#endif