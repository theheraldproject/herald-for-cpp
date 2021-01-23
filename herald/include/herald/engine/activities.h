//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "../context.h"
#include "../datatype/data.h"
#include "../datatype/target_identifier.h"

#include <memory>
#include <functional>
#include <optional>
#include <tuple>

namespace herald {
namespace engine {

using namespace herald::datatype;

// Types used in coordination of Herald sensor activities

using FeatureTag = herald::datatype::Data;

namespace Features {
  static FeatureTag HeraldBluetoothProtocolConnection = herald::datatype::Data(std::byte(0x01),1);
}

using Priority = std::uint8_t;

namespace Priorities {
  constexpr Priority Critical(200);
  constexpr Priority High(150);
  constexpr Priority Default(100);
  constexpr Priority Low(50);
}

struct Activity; // fwd decl

using Prerequisite = std::tuple<FeatureTag,std::optional<TargetIdentifier>>;
using PrioritisedPrerequisite = std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>;


// THE FOLLOWING IS FOR PLATFORMS WITH CALLBACK / STD::ASYNC+STD::FUTURE SUPPORT
/** callback for open and close connection **/
//using ConnectionCallback = std::function<void(const std::vector<PrioritisedPrerequisite> connectedTo)>;

/** Callback function or lambda provided by the Coordinator to be called once the action completes **/
//using CompletionCallback = std::function<void(const Activity,std::optional<Activity>)>; // function by value

/** Activity execution function or lambda **/
//using ActivityFunction = std::function<void(const Activity,CompletionCallback)>; // function by value

// THE FOLLOWING IS FOR PLATFORMS WITHOUT STD::ASYNC SUPPORT (i.e. that is SYNC ONLY)

using ActivityFunction = std::function<std::optional<Activity>(const Activity)>;

// END RESULTS ONLY PLATFORMS

struct Activity {
  Priority priority;
  std::string name;
  std::vector<Prerequisite> prerequisites; // no target id means all that are connected
  ActivityFunction executor;
};

/**
 * Some sensors may have dependencies on others, or system features.
 * This class provides a way of Sensors to let the Herald system know
 * of their requirements and capabilities at any given moment.
 */
class CoordinationProvider {
public:
  CoordinationProvider() = default;
  virtual ~CoordinationProvider() = default;
  
  // Coordination methods - Since v1.2-beta3
  /** What connections does this Sensor type provide for Coordination **/
  virtual std::vector<FeatureTag> connectionsProvided() = 0;

  // Runtime connection provisioning (if it isn't requested, it can be closed) **/
  // WITH STD::SYNC ONLY: virtual void provision(const std::vector<PrioritisedPrerequisite>& requested, const ConnectionCallback& connCallback) = 0;
  virtual std::vector<PrioritisedPrerequisite> provision(const std::vector<PrioritisedPrerequisite>& requested) = 0;

  // Runtime coordination callbacks
  /** Get a list of what connections are required to which devices now (may start, maintain, end (if not included)) **/
  virtual std::vector<PrioritisedPrerequisite> requiredConnections() = 0;
  virtual std::vector<Activity> requiredActivities() = 0;
};

}
}

#endif