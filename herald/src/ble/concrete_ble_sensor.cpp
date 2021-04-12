//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_concrete.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/ble/ble_coordinator.h"
#include "herald/ble/ble_sensor_configuration.h"
#include "herald/data/sensor_logger.h"

// C++17 includes
#include <memory>
#include <vector>
#include <optional>

namespace herald {
namespace ble {

using namespace herald::datatype;

// template <typename ContextT>
// class ConcreteBLESensor<ContextT>::Impl {
// public:
//   Impl(ContextT& ctx, 
//     BluetoothStateManager& bluetoothStateManager, 
//     std::shared_ptr<PayloadDataSupplier> payloadDataSupplier);
//   ~Impl();

//   // Internal API private methods here too

//   // Data members hidden by PIMPL

//   std::shared_ptr<ConcreteBLEDatabase<ContextT>> database;
//   BluetoothStateManager& stateManager;
//   std::shared_ptr<ConcreteBLETransmitter<ContextT>> transmitter;
//   std::shared_ptr<ConcreteBLEReceiver<ContextT>> receiver;

//   std::vector<std::shared_ptr<SensorDelegate>> delegates;
  
//   std::shared_ptr<HeraldProtocolBLECoordinationProvider<ContextT>> coordinator;

//   bool addedSelfAsDelegate;

//   HLOGGER(ContextT);
// };

// template <typename ContextT>
// ConcreteBLESensor<ContextT>::Impl::Impl(ContextT& ctx, 
//     BluetoothStateManager& bluetoothStateManager, 
//     std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
//   : database(std::make_shared<ConcreteBLEDatabase>(ctx)), 
//     stateManager(bluetoothStateManager),
//     transmitter(std::make_shared<ConcreteBLETransmitter>(
//       ctx, bluetoothStateManager, payloadDataSupplier, database)
//     ),
//     receiver(std::make_shared<ConcreteBLEReceiver>(
//       ctx, bluetoothStateManager, payloadDataSupplier, database)
//     ),
//     delegates(),
//     coordinator(std::make_shared<HeraldProtocolBLECoordinationProvider<ContextT>>(ctx, database, receiver)),
//     addedSelfAsDelegate(false)
//     HLOGGERINIT(ctx,"sensor","ConcreteBLESensor")
// {
//   ;
// }

// template <typename ContextT>
// ConcreteBLESensor<ContextT>::Impl::~Impl()
// {
//   ;
// }






// template <typename ContextT>
// ConcreteBLESensor<ContextT>::ConcreteBLESensor(ContextT& ctx, 
//     BluetoothStateManager& bluetoothStateManager, 
//     std::shared_ptr<PayloadDataSupplier> payloadDataSupplier)
//   : mImpl(std::make_unique<Impl>(ctx,bluetoothStateManager,payloadDataSupplier))
// {
//   ;
// }

// template <typename ContextT>
// ConcreteBLESensor<ContextT>::~ConcreteBLESensor()
// {
//   ;
// }

// template <typename ContextT>
// std::optional<std::shared_ptr<CoordinationProvider>>
// ConcreteBLESensor<ContextT>::coordinationProvider()
// {
//   // Only return this if we support scanning
//   if (BLESensorConfiguration::scanningEnabled) {
//     HDBG("Providing a BLECoordinationProvider");
//     return std::optional<std::shared_ptr<CoordinationProvider>>(mImpl->coordinator);
//   }
//   HDBG("Scanning not supported - so not returning a BLECoordinationProvider");
//   return {};
// }

// template <typename ContextT>
// bool
// ConcreteBLESensor<ContextT>::immediateSend(Data data, const TargetIdentifier& targetIdentifier)
// {
//   return mImpl->receiver->immediateSend(data,targetIdentifier);
// }

// template <typename ContextT>
// bool
// ConcreteBLESensor<ContextT>::immediateSendAll(Data data)
// {
//   return mImpl->receiver->immediateSendAll(data);
// }

// // Sensor overrides
// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::add(const std::shared_ptr<SensorDelegate>& delegate)
// {
//   mImpl->delegates.push_back(delegate);
//   // add all delegates to receiver and transmitter too?
//   mImpl->receiver->add(delegate);
//   mImpl->transmitter->add(delegate);
//   // TODO what about duplicates?
// }

// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::start()
// {
//   if (!mImpl->addedSelfAsDelegate) {
//     mImpl->stateManager.add(this->shared_from_this()); // FAILS IF USED IN THE CTOR - DO NOT DO THIS FROM CTOR
//     mImpl->database->add(this->shared_from_this());
//     mImpl->addedSelfAsDelegate = true;
//   }
//   mImpl->transmitter->start();
//   mImpl->receiver->start();
//   for (auto& delegate : mImpl->delegates) {
//     delegate->sensor(SensorType::BLE, SensorState::on);
//   }
// }

// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::stop()
// {
//   mImpl->transmitter->stop();
//   mImpl->receiver->stop();
//   for (auto& delegate : mImpl->delegates) {
//     delegate->sensor(SensorType::BLE, SensorState::off);
//   }
// }

// template <typename ContextT>
// // Database overrides
// void
// ConcreteBLESensor<ContextT>::bleDatabaseDidCreate(const std::shared_ptr<BLEDevice>& device)
// {
//   for (auto& delegate : mImpl->delegates) {
//     delegate->sensor(SensorType::BLE, device->identifier()); // didDetect
//   }
// }

// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::bleDatabaseDidUpdate(const std::shared_ptr<BLEDevice>& device, 
//   const BLEDeviceAttribute attribute)
// {
//   switch (attribute) {
//     case BLEDeviceAttribute::rssi: {
//       auto rssi = device->rssi();
//       if (rssi.has_value()) {
//         double rssiValue = (double)rssi->intValue();
//         auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
//         for (auto& delegate: mImpl->delegates) {
//           delegate->sensor(SensorType::BLE,
//             prox,
//             device->identifier()
//           ); // didMeasure
//         }
//         // also payload with rssi
//         auto payload = device->payloadData();
//         if (payload.has_value()) {
//           for (auto& delegate: mImpl->delegates) {
//             delegate->sensor(SensorType::BLE,
//               prox,
//               device->identifier(),
//               *payload
//             ); // didMeasure withPayload
//           }
//         }
//       }
//       break;
//     }
//     case BLEDeviceAttribute::payloadData: {
//       auto payload = device->payloadData();
//       if (payload.has_value()) {
//         for (auto& delegate: mImpl->delegates) {
//           delegate->sensor(SensorType::BLE,
//             *payload,
//             device->identifier()
//           ); // didReadPayload
//         }
//         // also payload with rssi
//         auto rssi = device->rssi();
//         if (rssi.has_value()) {
//           double rssiValue = (double)rssi->intValue();
//           auto prox = Proximity{.unit=ProximityMeasurementUnit::RSSI, .value=rssiValue};
//           for (auto& delegate: mImpl->delegates) {
//             delegate->sensor(SensorType::BLE,
//               prox,
//               device->identifier(),
//               *payload
//             ); // didMeasure withPayload
//           }
//         }
//       }
//       break;
//     }
//     default: {
//       ; // do nothing
//     }
//   }
// }

// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::bleDatabaseDidDelete(const std::shared_ptr<BLEDevice>& device)
// {
//   ; // TODO just log this // TODO determine if to pass this on too
// }

// // Bluetooth state manager delegate overrides
// template <typename ContextT>
// void
// ConcreteBLESensor<ContextT>::bluetoothStateManager(BluetoothState didUpdateState)
// {
//   if (BluetoothState::poweredOff == didUpdateState) {
//     // stop();
//   }
//   if (BluetoothState::poweredOn == didUpdateState) {
//     // start();
//   }
//   if (BluetoothState::unsupported == didUpdateState) {
//     for (auto& delegate : mImpl->delegates) {
//       delegate->sensor(SensorType::BLE, SensorState::unavailable);
//     }
//   }
// }

}
}