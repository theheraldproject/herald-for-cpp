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

template <typename... SensorDelegateTs>
class SensorDelegateSet {
public:
  static constexpr std::size_t Size = sizeof...(SensorDelegateTs);

  SensorDelegateSet(SensorDelegateTs&... dels) : delegates() {
    addDelegates(0,dels...);
  }

  /// Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) {
    
  }

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) {
    for (auto& delegateV : delegates) {
      std::visit([sensor,didRead,fromTarget](auto&& arg) {
        // using noref = typename std::remove_reference<decltype(arg)>::type;
        // if constexpr (std::is_same_v<ValT,typename noref::value_type>) {
          ((decltype(arg))arg).get().sensor(sensor,didRead,fromTarget); // cast to call derived class function
        // }
      }, delegateV);
    }
  }

  /// Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) {

  }

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, const DataSections<8>& didShare, const TargetIdentifier& fromTarget) {

  }

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) {

  }

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) {

  }

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) {

  }

  /// Sensor state update
  void sensor(SensorType sensor, const SensorState& didUpdateState) {

  }

private:
  std::array<std::variant<std::reference_wrapper<SensorDelegateTs...>>,Size> delegates;

  template <typename LastT>
  constexpr void addDelegates(int nextPos,LastT& last) {
    delegates[nextPos] = std::reference_wrapper<LastT>(last);
  }

  template <typename FirstT, typename SecondT, typename... RestT>
  constexpr void addDelegates(int nextPos,FirstT& first, SecondT& second, RestT&... rest) {
    delegates[nextPos] = std::reference_wrapper<FirstT>(first);
    ++nextPos;
    addDelegates(nextPos,second,rest...);
  }
};


} // end namespace

#endif