//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/ble.h"

#include <utility> // C++17 std::as_const

namespace herald {
namespace ble {

using namespace herald::datatype;

BLECharacteristicType::BLECharacteristicType() noexcept
  : m_type()
{
  m_type.set(0,true); // Default to read characteristic
}

BLECharacteristicType::BLECharacteristicType(const BLECharacteristicTypeValue& value) noexcept
  : m_type()
{
  *this = value; // call assign operator
}

BLECharacteristicType&
operator|=(BLECharacteristicType& toUpdate, const BLECharacteristicTypeValue& from) noexcept
{
  switch (from) {
    case BLECharacteristicTypeValue::WriteWithoutAck:
      toUpdate.m_type.set(1,true);
      break;
    case BLECharacteristicTypeValue::WriteWithAck:
      toUpdate.m_type.set(2,true);
      break;
    case BLECharacteristicTypeValue::Notify:
      toUpdate.m_type.set(3,true);
      break;
    case BLECharacteristicTypeValue::Read:
    default:
      toUpdate.m_type.set(0,true);
      break;
  }
  return toUpdate;
}

BLECharacteristicType&
operator|=(BLECharacteristicType& toUpdate, const BLECharacteristicType& from) noexcept
{
  toUpdate.m_type |= from.m_type;
  return toUpdate;
}

bool
BLECharacteristicType::operator==(const BLECharacteristicTypeValue& value) const noexcept
{
  switch (value) {
    case BLECharacteristicTypeValue::WriteWithoutAck:
      return m_type.test(1);
    case BLECharacteristicTypeValue::WriteWithAck:
      return m_type.test(2);
    case BLECharacteristicTypeValue::Notify:
      return m_type.test(3);
    case BLECharacteristicTypeValue::Read:
      return m_type.test(0);
    default:
      return false;
  }
}

bool
BLECharacteristicType::operator!=(const BLECharacteristicTypeValue& value) const noexcept
{
  return !(*this == value);
}

bool
BLECharacteristicType::operator==(const BLECharacteristicType& other) const noexcept
{
  return m_type == other.m_type;
}

bool
BLECharacteristicType::operator!=(const BLECharacteristicType& other) const noexcept
{
  return m_type != other.m_type;
}




BluetoothUUID::BluetoothUUID() noexcept
  : m_size(BluetoothUUIDSize::Empty),
    m_data()
{
  ;
}

BluetoothUUID::BluetoothUUID(Data&& from) noexcept
  : m_size(BluetoothUUIDSize::Empty),
    m_data(from) 
{
  auto sz = m_data.size();
  if (sz < 2) {
    m_size = BluetoothUUIDSize::Empty;
  }
  if (sz < 4) {
    m_size = BluetoothUUIDSize::Short16;
  }
  if (sz < 8) {
    m_size = BluetoothUUIDSize::Medium32;
  }
  if (sz < 16) {
    m_size = BluetoothUUIDSize::Long64;
  }
  m_size = BluetoothUUIDSize::Full128;
}

BluetoothUUID::BluetoothUUID(const std::size_t sz) noexcept
  : m_size(BluetoothUUIDSize::Empty),
    m_data(sz)
{
  if (sz < 2) {
    m_size = BluetoothUUIDSize::Empty;
  }
  if (sz < 4) {
    m_size = BluetoothUUIDSize::Short16;
  }
  if (sz < 8) {
    m_size = BluetoothUUIDSize::Medium32;
  }
  if (sz < 16) {
    m_size = BluetoothUUIDSize::Long64;
  }
  m_size = BluetoothUUIDSize::Full128;
}

BluetoothUUID::operator std::size_t() const noexcept
{
  switch (m_size) {
    case BluetoothUUIDSize::Short16:
      return 2; // 2 bytes = 16 bits
    case BluetoothUUIDSize::Medium32:
      return 4;
    case BluetoothUUIDSize::Long64:
      return 8;
    case BluetoothUUIDSize::Full128:
      return 16;
    case BluetoothUUIDSize::Empty:
    default:
      return 0;
  }
}

BluetoothUUIDSize
BluetoothUUID::size() const noexcept
{
  return m_size;
}

const Data&
BluetoothUUID::value() const noexcept
{
  return m_data;
}

bool
BluetoothUUID::operator==(const BluetoothUUID& other) const noexcept
{
  return m_size == other.m_size && m_data == other.m_data;
}

bool
BluetoothUUID::operator!=(const BluetoothUUID& other) const noexcept
{
  return m_size != other.m_size || m_data != other.m_data;
}





BLECharacteristic::BLECharacteristic() noexcept
  : uuid(),
    type(),
    callbacks()
{
  ;
}

BLECharacteristic::BLECharacteristic(BluetoothUUID id, BLECharacteristicType ctype, BLECallbacks cbs) noexcept
  : uuid(id),
    type(ctype),
    callbacks(cbs)
{
  ;
}

BLECharacteristic::BLECharacteristic(BLECharacteristic&& other) noexcept
  : uuid(std::move(other.uuid)),
    type(std::move(other.type)),
    callbacks(std::move(other.callbacks))
{
  ;
}

BLECharacteristic::BLECharacteristic(const BLECharacteristic& other) noexcept 
  : uuid(other.uuid),
    type(other.type),
    callbacks(other.callbacks)
{
  ;
}

BLECharacteristic::operator BluetoothUUID() const noexcept
{
  return uuid;
}

BLECharacteristic&
BLECharacteristic::operator=(const BLECharacteristic& from)
{
  uuid = from.uuid;
  type = from.type;
  callbacks = from.callbacks;
  return *this;
}

BLECharacteristic&
BLECharacteristic::operator=(BLECharacteristic&& from)
{
  uuid = std::move(from.uuid);
  type = std::move(from.type);
  callbacks = std::move(from.callbacks);
  return *this;
}

bool
BLECharacteristic::operator==(const BLECharacteristic& other) const noexcept
{
  // TODO check for characteristic type equality too
  return uuid == other.uuid;
}

bool
BLECharacteristic::operator!=(const BLECharacteristic& other) const noexcept
{
  // TODO check for characteristic type equality too
  return uuid != other.uuid;
}

bool
BLECharacteristic::operator==(const BluetoothUUID& id) const noexcept
{
  return uuid == id;
}

bool
BLECharacteristic::operator!=(const BluetoothUUID& id) const noexcept
{
  return uuid == id;
}

bool
BLECharacteristic::operator==(const BLECharacteristicType& characteristicType) const noexcept
{
  return type == characteristicType;
}

bool
BLECharacteristic::operator!=(const BLECharacteristicType& characteristicType) const noexcept
{
  return type != characteristicType;
}


BLECharacteristic&
operator|=(BLECharacteristic& toUpdate, const BLECharacteristic& toMerge) noexcept
{
  toUpdate.type |= toMerge.type;
  return toUpdate;
}

BLECharacteristic&
operator|=(BLECharacteristic& toUpdate, const BLECharacteristicType& toMerge) noexcept
{
  toUpdate.type |= toMerge;
  return toUpdate;
}











BLEService::BLEService() noexcept
  : uuid(),
    characteristics()
{
  ;
}

BLEService::BLEService(BluetoothUUID id,BLECharacteristicList cl) noexcept
  : uuid(id),
    characteristics(cl)
{
  ;
}

BLEService::BLEService(BLEService&& other) noexcept
  : uuid(std::move(other.uuid)),
    characteristics(std::move(other.characteristics))
{
  ;
}

BLEService::BLEService(const BLEService& other) noexcept
  : uuid(other.uuid),
    characteristics(other.characteristics)
{
  ;
}

BLEService::operator BluetoothUUID() const noexcept
{
  return uuid;
}

BLEService&
BLEService::operator=(const BLEService& from) noexcept
{
  uuid = from.uuid;
  characteristics = from.characteristics;
  return *this;
}

BLEService&
BLEService::operator=(BLEService&& from) noexcept
{
  uuid = std::move(from.uuid);
  characteristics = std::move(from.characteristics);
  return *this;
}

bool
BLEService::operator==(const BluetoothUUID& id) const noexcept
{
  return uuid == id;
}

bool
BLEService::operator!=(const BluetoothUUID& id) const noexcept
{
  return uuid != id;
}

bool
BLEService::operator==(const BLEService& other) const noexcept
{
  // TODO check characteristic values too
  return uuid == other.uuid;
}

bool
BLEService::operator!=(const BLEService& other) const noexcept
{
  // TODO check characteristic values too
  return uuid != other.uuid;
}


BLEService&
operator|=(BLEService& toUpdate, BLEService& toMerge) noexcept
{
  // for each characteristic, either merge a matching char or add one
  // for(auto chit = toMerge.characteristics.cbegin(), chend = toMerge.characteristics.cend();chit != chend;++chit) {
  for (auto& ch : toMerge.characteristics) {
    bool found = false;
    // for (auto it = toUpdate.characteristics.begin(), end = toUpdate.characteristics.end(); it != end;++it) {
    for (auto& mych : toUpdate.characteristics) {
      if (mych == ch) {
        found = true;
        mych |= ch;
        continue;
      }
    }
    if (!found) {
      toUpdate.characteristics.add(ch);
    }
  }
  return toUpdate;
}

BLEService&
operator|=(BLEService& toUpdate, const BLECharacteristic& toMerge) noexcept
{
  for (auto& mych : toUpdate.characteristics) {
    if (mych == toMerge) {
      mych |= toMerge;
      return toUpdate;
    }
  }
  toUpdate.characteristics.add(toMerge);
  return toUpdate;
}

}
}