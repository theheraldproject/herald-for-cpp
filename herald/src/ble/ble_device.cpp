//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble_device.h"

#include "herald/datatype/date.h"
#include "herald/datatype/time_interval.h"
#include "herald/datatype/target_identifier.h"

#include <optional>

namespace herald {
namespace ble {

class BLEDevice::Impl {
public:
  Impl(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> del);
  ~Impl();

  TargetIdentifier id;
  std::shared_ptr<BLEDeviceDelegate> delegate;

  // Data holders
  Date created;
  std::optional<Date> lastUpdated;
};

BLEDevice::Impl::Impl(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> del)
  : id(identifier),
    delegate(del),
    created(Date()),
    lastUpdated(std::optional<Date>())
{
  ;
}

BLEDevice::Impl::~Impl()
{
  ;
}






BLEDevice::BLEDevice(TargetIdentifier identifier, std::shared_ptr<BLEDeviceDelegate> delegate)
  : Device(),
    mImpl(std::make_unique<Impl>(identifier,delegate))
{
  ;
}

BLEDevice::~BLEDevice()
{
  ;
}


const TargetIdentifier&
BLEDevice::identifier() const
{
  return mImpl->id;
}


// basic descriptors
std::string
BLEDevice::description() const
{
  return "";
}

BLEDevice::operator std::string() const
{
  return "";
}

// timing related getters
std::optional<TimeInterval>
BLEDevice::timeIntervalSinceLastUpdate() const
{
  if (!mImpl->lastUpdated.has_value()) {
    return std::optional<TimeInterval>();;
  } // TODO send back inf time instead?
  return std::optional<TimeInterval>(Date() - mImpl->lastUpdated.value());
}

std::optional<TimeInterval>
BLEDevice::timeIntervalSinceConnected() const
{
  return std::optional<TimeInterval>();
}
std::optional<TimeInterval>
BLEDevice::timeIntervalSinceLastPayloadDataUpdate() const
{
  return std::optional<TimeInterval>();
}
std::optional<TimeInterval>
BLEDevice::timeIntervalSinceLastWritePayloadSharing() const
{
  return std::optional<TimeInterval>();
}
std::optional<TimeInterval>
BLEDevice::timeIntervalSinceLastWritePayload() const
{
  return std::optional<TimeInterval>();
}
std::optional<TimeInterval>
BLEDevice::timeIntervalSinceLastWriteRssi() const
{
  return std::optional<TimeInterval>();
}

// property getters and setters
//std::optional<PseudoDeviceAddress> pseudoDeviceAddress() const;
//void pseudoDeviceAddress(PseudoDeviceAddress newAddress);
//std::optional<BluetothDevice> peripheral() const;
//void peripheral(BluetoothDevice newPeripheral);
std::optional<BLEDeviceState>
BLEDevice::state() const
{
  return BLEDeviceState::disconnected;
}

void
BLEDevice::state(BLEDeviceState newState)
{

}

std::optional<BLEDeviceOperatingSystem>
BLEDevice::operatingSystem() const
{
  return std::optional<BLEDeviceOperatingSystem>(BLEDeviceOperatingSystem::unknown);
}

void
BLEDevice::operatingSystem(BLEDeviceOperatingSystem newOS)
{

}

std::optional<PayloadData>
BLEDevice::payloadData() const
{
  return std::optional<PayloadData>();
}

void
BLEDevice::payloadData(PayloadData newPayloadData)
{

}

std::optional<ImmediateSendData>
BLEDevice::immediateSendData() const
{
  return std::optional<ImmediateSendData>();
}

void
BLEDevice::immediateSendData(ImmediateSendData toSend)
{

}

std::optional<RSSI>
BLEDevice::rssi() const
{
  return std::optional<RSSI>();
}
void
BLEDevice::rssi(RSSI newRSSI)
{

}

//std::optional<BLETxPower> txPower() const;
//void txPower(BLETxPower newPower);

bool
BLEDevice::receiveOnly() const
{
  return false;
}

void
BLEDevice::receiveOnly(bool newReceiveOnly)
{

}

//std::optional<BluetoothGattCharacteristic> signalCharacteristic() const; // TODO local OS alternative
//void signalCharacteristic(BluetoothGattCharacteristic newChar);

//std::optional<BluetoothGattCharacteristic> payloadCharacteristic() const; // TODO local OS alternative
//void payloadCharacteristic(BluetoothGattCharacteristic newChar);

// State engine methods
bool
BLEDevice::ignore() const
{
  return false;
}

void
BLEDevice::invalidateCharacteristics()
{

}

void
BLEDevice::registerDiscovery(Date& at)
{

}

void
BLEDevice::registerWritePayload(Date& at)
{

}

void
BLEDevice::registerWritePayloadSharing(Date& at)
{

}

void
BLEDevice::registerWriteRssi(Date& at)
{

}


}
}