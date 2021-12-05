//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SENSOR_DELEGATE_H
#define HERALD_SENSOR_DELEGATE_H

#include "datatype/sensor_type.h"
#include "datatype/proximity.h"
#include "datatype/payload_data.h"
#include "datatype/target_identifier.h"
#include "datatype/immediate_send_data.h"
#include "datatype/location.h"
#include "datatype/sensor_state.h"
#include "util/is_valid.h"

#include <variant>
#include <array>

namespace herald {
  
using namespace datatype;

// /// \brief Base interface for classes wishing to implement callbacks for core low-level Herald proximity and presence events.
// class SensorDelegate {
// public:
//   SensorDelegate() = default;
//   virtual ~SensorDelegate() = default;

  
//   /// Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
//   virtual void sensor(SensorType sensor, const TargetIdentifier& didDetect) = 0;

//   /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
//   virtual void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) = 0;

//   /// Receive written immediate send data from target, e.g. important timing signal.
//   virtual void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) = 0;

//   /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
//   virtual void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) = 0;

//   /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
//   virtual void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) = 0;

//   /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
//   template <typename LocationT>
//   void sensor(SensorType sensor, const Location<LocationT>& didVisit) {}

//   /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
//   virtual void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) = 0;

//   /// Sensor state update
//   virtual void sensor(SensorType sensor, const SensorState& didUpdateState) = 0;
// };

constexpr auto hasSensorFunction = herald::util::isValid(
  [](auto&& s,auto&& sensor,auto&& didRead,auto&& fromTarget) ->
    decltype(((decltype(s))s).get().sensor(sensor,didRead,fromTarget)) {}
);
// template<typename T,typename SE, typename DR, typename FT>
// using HasSensorFunctionT = decltype(hasSensorFunction(std::declval<T>(),std::declval<SE>(),std::declval<DR>(),std::declval<FT>()));
template<typename T,typename... Args>
using HasSensorFunctionT = decltype(hasSensorFunction(std::declval<T>(),std::declval<Args>()...));
template<typename T,typename... Args>
constexpr auto HasSensorFunctionV = HasSensorFunctionT<T,Args...>::value;

/// \brief A set of variant typed Sensor Delegate instances. Delegate callbacks can be invoked on the whole set, if supported by each.
template <typename... SensorDelegateTs>
class SensorDelegateSet {
public:
  static constexpr std::size_t Size = sizeof...(SensorDelegateTs);

  SensorDelegateSet(SensorDelegateTs&... dels) noexcept : 
    //delegates({std::variant<std::reference_wrapper<SensorDelegateTs&...>>(std::reference_wrapper<SensorDelegateTs&...>(dels...))}) {
    // delegates(std::experimental::make_array({std::reference_wrapper<SensorDelegateTs&...>(dels...)})) {
    delegates(std::array<
      std::variant<std::reference_wrapper<SensorDelegateTs>...>
      ,Size
    >({std::variant<std::reference_wrapper<SensorDelegateTs>...>(dels)...})) {
    // addDelegates(0,dels...);
  }
  ~SensorDelegateSet() noexcept = default;

  // auto begin() -> decltype(std::declval<std::array<
  //     std::variant<std::reference_wrapper<SensorDelegateTs>...>
  //     ,Size
  //   >>().begin()) {
  //   return delegates.begin();
  // }

  // auto end() -> decltype(std::declval<std::array<
  //     std::variant<std::reference_wrapper<SensorDelegateTs>...>
  //     ,Size
  //   >>().end()) {
  //   return delegates.end();
  // }

  /// \brief Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) noexcept {
        for (auto& delegateV : delegates) {
      std::visit([sensor,didDetect](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didDetect)>) {
          ((decltype(arg))arg).get().sensor(sensor,didDetect); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didRead,fromTarget](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didRead),decltype(fromTarget)>) {
          ((decltype(arg))arg).get().sensor(sensor,didRead,fromTarget); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didReceive,fromTarget](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didReceive),decltype(fromTarget)>) {
          ((decltype(arg))arg).get().sensor(sensor,didReceive,fromTarget); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, const DataSections<8>& didShare, const TargetIdentifier& fromTarget) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didShare,fromTarget](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didShare),decltype(fromTarget)>) {
          ((decltype(arg))arg).get().sensor(sensor,didShare,fromTarget); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didMeasure,fromTarget](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didMeasure),decltype(fromTarget)>) {
          ((decltype(arg))arg).get().sensor(sensor,didMeasure,fromTarget); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didVisit](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didVisit)>) {
          ((decltype(arg))arg).get().sensor(sensor,didVisit); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didMeasure,fromTarget,withPayload](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didMeasure),decltype(fromTarget),decltype(withPayload)>) {
          ((decltype(arg))arg).get().sensor(sensor,didMeasure,fromTarget,withPayload); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// \brief Sensor state update
  void sensor(SensorType sensor, const SensorState& didUpdateState) noexcept {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didUpdateState](auto&& arg) {
        // ONLY INVOKE IF WE KNOW THIS FUNCTION EXISTS
        if constexpr (HasSensorFunctionV<decltype(arg),decltype(sensor),decltype(didUpdateState)>) {
          ((decltype(arg))arg).get().sensor(sensor,didUpdateState); // cast to call derived class function
        }
      }, delegateV);
    }
  }

private:
  std::array<
    std::variant<std::reference_wrapper<SensorDelegateTs>...>
    ,
    Size
  > delegates;

  // template <typename LastT>
  // constexpr void addDelegates(int nextPos,LastT& last) {
  //   delegates[nextPos] = std::reference_wrapper<LastT>(last);
  // }

  // template <typename FirstT, typename SecondT, typename... RestT>
  // constexpr void addDelegates(int nextPos,FirstT& first, SecondT& second, RestT&... rest) {
  //   delegates[nextPos] = std::reference_wrapper<FirstT>(first);
  //   ++nextPos;
  //   addDelegates(nextPos,second,rest...);
  // }
};


} // end namespace

#endif