//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/bluetooth_state_manager.h"

namespace herald {
namespace ble {

using namespace herald::datatype;

CharacteristicType::CharacteristicType()
  : m_type()
{
  m_type.set(0,true); // Default to read characteristic
}

CharacteristicType::CharacteristicType(const CharacteristicTypeValue& value)
  : m_type()
{
  *this = value; // call assign operator
}

CharacteristicType&
CharacteristicType::operator=(const CharacteristicTypeValue& from)
{
  switch (from) {
    case CharacteristicTypeValue::WRITE_WITHOUT_ACK:
      m_type.set(1,true);
      break;
    case CharacteristicTypeValue::WRITE_WITH_ACK:
      m_type.set(2,true);
      break;
    case CharacteristicTypeValue::NOTIFY:
      m_type.set(3,true);
      break;
    case CharacteristicTypeValue::READ:
    default:
      m_type.set(0,true);
      break;
  }
  return *this;
}

bool
CharacteristicType::operator==(const CharacteristicTypeValue& value) const
{
  switch (value) {
    case CharacteristicTypeValue::WRITE_WITHOUT_ACK:
      return m_type.test(1);
    case CharacteristicTypeValue::WRITE_WITH_ACK:
      return m_type.test(2);
    case CharacteristicTypeValue::NOTIFY:
      return m_type.test(3);
    case CharacteristicTypeValue::READ:
      return m_type.test(0);
    default:
      return false;
  }
}

bool
CharacteristicType::operator!=(const CharacteristicTypeValue& value) const
{
  return !(*this == value);
}






BluetoothUUID::BluetoothUUID(Data&& from)
  : m_size(BluetoothUUIDSize::EMPTY),
    m_data(from) 
{
  auto sz = m_data.size();
  if (sz < 2) {
    m_size = BluetoothUUIDSize::EMPTY;
  }
  if (sz < 4) {
    m_size = BluetoothUUIDSize::SHORT_16;
  }
  if (sz < 8) {
    m_size = BluetoothUUIDSize::MEDIUM_32;
  }
  if (sz < 16) {
    m_size = BluetoothUUIDSize::LONG_64;
  }
  m_size = BluetoothUUIDSize::FULL_128;
}

BluetoothUUID::BluetoothUUID(const std::size_t sz)
  : m_size(BluetoothUUIDSize::EMPTY),
    m_data(sz)
{
  if (sz < 2) {
    m_size = BluetoothUUIDSize::EMPTY;
  }
  if (sz < 4) {
    m_size = BluetoothUUIDSize::SHORT_16;
  }
  if (sz < 8) {
    m_size = BluetoothUUIDSize::MEDIUM_32;
  }
  if (sz < 16) {
    m_size = BluetoothUUIDSize::LONG_64;
  }
  m_size = BluetoothUUIDSize::FULL_128;
}

BluetoothUUID::operator std::size_t() const
{
  switch (m_size) {
    case BluetoothUUIDSize::SHORT_16:
      return 2; // 2 bytes = 16 bits
    case BluetoothUUIDSize::MEDIUM_32:
      return 4;
    case BluetoothUUIDSize::LONG_64:
      return 8;
    case BluetoothUUIDSize::FULL_128:
      return 16;
    case BluetoothUUIDSize::EMPTY:
    default:
      return 0;
  }
}

BluetoothUUIDSize
BluetoothUUID::size() const
{
  return m_size;
}

const Data&
BluetoothUUID::value() const
{
  return m_data;
}


}
}