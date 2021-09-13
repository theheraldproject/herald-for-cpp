//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_NORDIC_UART_SENSOR_DELEGATE_H
#define HERALD_NORDIC_UART_SENSOR_DELEGATE_H

#include "../../../sensor_delegate.h"

#ifdef __ZEPHYR__
// #ifdef BT_NUS
#include <bluetooth/bluetooth.h>
#include <bluetooth/services/nus.h>
// #endif
#endif

#include <functional>

namespace herald {
namespace ble {
// Platforms (E.g. Zephyr) don't have namespaces included for them, so proceed straight to nordic_uart
namespace nordic_uart {

/// \brief Compatible platform specific sink functions (E.g. bt_nus_send on Zephyr with NCS v1.6.1)
/// \since v2.1.0
using SendFunction = std::function<void(void*,const char*,std::size_t)>;

const SendFunction _DefaultSendFunction = 
#ifdef __ZEPHYR__
// #ifdef BT_NUS
  [](void* vp,const char* cp,std::size_t l) {bt_nus_send((bt_conn*)vp,(const unsigned char*)cp,l);};
// #else
//   [](void* vp,const char* cp,std::size_t l) {};
// #endif
#else
  [](void* vp,const char* cp,std::size_t l) {};
#endif
;

/// \brief Sensor Delegate that transmits all callbacks as CSV over the Nordic Bluetooth UART service (or similar callback mechanism)
/// \since v2.1.0
template <typename ContextT>
class NordicUartSensorDelegate {
public:
  NordicUartSensorDelegate(ContextT& context,SendFunction sendFunc = _DefaultSendFunction)
    : ctx(context),
      buffer("SensorDelegate,"),
      position(0),
      sender(sendFunc)
  {
    ; // TODO ensure our NUS service is advertised appropriately by BLETransmitter
  }
  ~NordicUartSensorDelegate() = default;

  // MARK: SENSOR DELEGATE PUBLIC METHODS
  
  /// \brief Detection of a target with an ephemeral identifier, e.g. BLE central detecting a BLE peripheral.
  void sensor(SensorType sensor, const TargetIdentifier& didDetect) {
    newline("didDetect");
    column((std::string)didDetect);
    sendline();
  }

  /// \brief Measure proximity to target
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) {
    newline("didMeasure");
    column((std::string)fromTarget);
    column(std::to_string((int)didMeasure.value));
    sendline();
  }

  /// \brief Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) {
    newline("didMeasureWithPayload");
    column((std::string)fromTarget);
    column(std::to_string((int)didMeasure.value));
    column(withPayload.hexEncodedString());
    sendline();
  }

private:
  ContextT& ctx;
  char buffer[128];
  std::size_t position;
  SendFunction sender;

  void newline(const char* callbackType) {
    position = 15;
    while (position < 126 && *callbackType != '\0') {
      buffer[position++] = *callbackType;
      ++callbackType;
    }
  }

  void column(const Data& value) {
    if (position >= 126) {
      return; // safety
    }
    buffer[position++] = ',';
    const std::size_t l = value.size();
    std::size_t dp = 0;
    while (position < 126 && dp < l) {
      buffer[position++] = (char)value.at(dp);
      ++dp;
    }
  }

  void column(const char* value) {
    if (position >= 126) {
      return; // safety
    }
    buffer[position++] = ',';
    while (position < 126 && *value != '\0') {
      buffer[position++] = *value;
      ++value;
    }
  }

  void sendline() {
    if (position >= 126) {
      buffer[126] = '\n';
      buffer[127] = '\0';
      position = 128;
    } else {
      buffer[position++] = '\n';
      buffer[position++] = '\0';
    }
    send(position);
    position = 0;
  }

  void send(std::size_t len) {
    sender(NULL, buffer, len);
  }
};

}
}
}

#endif