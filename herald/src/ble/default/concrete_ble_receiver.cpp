//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/data/sensor_logger.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_receiver.h"

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::data;

// /**
//  * Dummy BLE Receiver to allow compilation on Zephyr and other platforms
//  * when Receiver code (E.g. OS scan libraries) have been compiled out.
//  * See herald.cmake for usage.
//  */

// class ConcreteBLEReceiver::Impl {
// public:
//   Impl() = default;
//   ~Impl() = default;
// };



// ConcreteBLEReceiver::ConcreteBLEReceiver(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
//   std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
//   : mImpl(std::make_unique<Impl>())
// {
//   ;
// }

// ConcreteBLEReceiver::~ConcreteBLEReceiver() = default;


// // Coordination overrides - Since v1.2-beta3
// std::optional<std::shared_ptr<CoordinationProvider>>
// ConcreteBLEReceiver::coordinationProvider()
// {
//   return {};
// }

// bool
// ConcreteBLEReceiver::immediateSend(Data data, const TargetIdentifier& targetIdentifier)
// {
//   return false;
// }

// bool
// ConcreteBLEReceiver::immediateSendAll(Data data)
// {
//   return false;
// }


// // Sensor overrides
// void
// ConcreteBLEReceiver::add(const std::shared_ptr<SensorDelegate>& delegate)
// {
//   ;
// }

// void
// ConcreteBLEReceiver::start()
// {
//   ;
// }

// void
// ConcreteBLEReceiver::stop()
// {
//   ;
// }


// bool
// ConcreteBLEReceiver::openConnection(const TargetIdentifier& toTarget)
// {
//   return false;
// }

// bool
// ConcreteBLEReceiver::closeConnection(const TargetIdentifier& toTarget)
// {
//   return false;
// }

// void
// ConcreteBLEReceiver::restartScanningAndAdvertising()
// {
//   ;
// }

// std::optional<Activity>
// ConcreteBLEReceiver::serviceDiscovery(Activity)
// {
//   return {};
// }

// std::optional<Activity>
// ConcreteBLEReceiver::readPayload(Activity)
// {
//   return {};
// }

// std::optional<Activity>
// ConcreteBLEReceiver::immediateSend(Activity)
// {
//   return {};
// }

// std::optional<Activity>
// ConcreteBLEReceiver::immediateSendAll(Activity)
// {
//   return {};
// }

}
}
