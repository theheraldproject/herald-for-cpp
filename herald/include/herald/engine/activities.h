//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ACTIVITIES_H
#define HERALD_ACTIVITIES_H

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

/// \brief Herald implementation provided Feature tag/identifier
using FeatureTag = herald::datatype::Data;

/// \brief Lists all Features currently supported by Herald providers
namespace Features {
  /// \brief Herald Bluetooth protocol connection is the first supported dependency type.
  static FeatureTag HeraldBluetoothProtocolConnection = herald::datatype::Data(std::byte(0x01),1);
}

/// \brief A relative priority between different activities
using Priority = std::uint8_t;

/// \brief Convenience Priority values
///
/// Try to not use the same value, but rather offset by 5 or 10 between different dependencies.
/// Be sure to avoid circular priority dependencies.
namespace Priorities {
  constexpr Priority Critical(200);
  constexpr Priority High(150);
  constexpr Priority Default(100);
  constexpr Priority Low(50);
}

struct Activity; // fwd decl

/// \brief An absolute prerequisite required before an activity can take place
///
/// An example would be the presence of a Bluetooth connection to a specified Device.
using Prerequisite = std::tuple<FeatureTag,std::optional<TargetIdentifier>>;
/// \brief a Presrequisite with a relative priority assigned to assist Herald to prioritise effectively.
using PrioritisedPrerequisite = std::tuple<FeatureTag,Priority,std::optional<TargetIdentifier>>;


// THE FOLLOWING IS FOR PLATFORMS WITH CALLBACK / STD::ASYNC+STD::FUTURE SUPPORT
/** callback for open and close connection **/
//using ConnectionCallback = std::function<void(const std::vector<PrioritisedPrerequisite> connectedTo)>;

/** Callback function or lambda provided by the Coordinator to be called once the action completes **/
//using CompletionCallback = std::function<void(const Activity,std::optional<Activity>)>; // function by value

/** Activity execution function or lambda **/
//using ActivityFunction = std::function<void(const Activity,CompletionCallback)>; // function by value

// THE FOLLOWING IS FOR PLATFORMS WITHOUT STD::ASYNC SUPPORT (i.e. that are SYNC ONLY)

/// \brief A convenience Function alias that invokes an activity and optionally returns a follow-on activity.
using ActivityFunction = std::function<std::optional<Activity>(const Activity)>;

// END RESULTS ONLY PLATFORMS

/// \brief An activity that needs to be performed due to some state being achieved in a Sensor
struct Activity {
  /// \brief The relative priority for this Activity compared to the scale
  /// \sa Priorities
  Priority priority;
  /// \brief A human readable name for this activity used for logging.
  std::string name;
  /// \brief A list of non-prioritised pre-requisities (priority is taken from the priority field in Activity).
  std::vector<Prerequisite> prerequisites; // no target id means all that are connected
  /// \brief The Activity function to call when all prerequisites have been met. May not be called.
  ActivityFunction executor;
};

/// \brief Coordination management class that arranges Sensor's periodic requirements and activity interdependencies.
///
/// Some sensors may have dependencies on others, or system features.
/// This class provides a way of Sensors to let the Herald system know
/// of their requirements and capabilities at any given moment.
///
class CoordinationProvider {
public:
  CoordinationProvider() = default;
  virtual ~CoordinationProvider() = default;
  
  // Coordination methods - Since v1.2-beta3
  /// What connections does this Sensor type provide for Coordination
  virtual std::vector<FeatureTag> connectionsProvided() = 0;

  /// \brief Runtime connection provisioning (if it isn't requested, it can be closed)
  ///
  /// Note:  WITH STD::SYNC ONLY: virtual void provision(const std::vector<PrioritisedPrerequisite>& requested, const ConnectionCallback& connCallback) = 0;
  virtual std::vector<PrioritisedPrerequisite> provision(const std::vector<PrioritisedPrerequisite>& requested) = 0;

  // Runtime coordination callbacks
  /// \brief Get a list of what connections are required to which devices now (may start, maintain, end (if not included))
  virtual std::vector<PrioritisedPrerequisite> requiredConnections() = 0;
  /// \brief Get a list of activities that are currently outstanding in this iteration
  virtual std::vector<Activity> requiredActivities() = 0;
};

}
}

#endif