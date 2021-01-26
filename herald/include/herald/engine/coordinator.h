//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "../context.h"
#include "../sensor.h"

#include <memory>

namespace herald {
namespace engine {

/**
 * Coordinates all connection and activities used across all sensors within Herald
 * 
 * Responsible for:-
 * - Determining Sensor capabilities and requirements around connections and activities
 * 
 * Manages:- 
 * - Nothing, but coordinates activities throughout Herald Sensor networks on behalf of Sensor Array
 * 
 * Is managed by:-
 * - Sensor Array
 */
class Coordinator {
public:
  Coordinator(std::shared_ptr<Context> context);
  ~Coordinator();

  /** Introspect and include in iteration planning **/
  void add(std::shared_ptr<Sensor> sensor);
  /** Remove from iteration planning **/
  void remove(std::shared_ptr<Sensor> sensor);

  /** Prepares for iterations to be called (may pre-emptively make calls) **/
  void start();
  /** Execute an iteration of activity, according to settings **/
  void iteration();
  /** Closes out any existing connections/activities **/
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