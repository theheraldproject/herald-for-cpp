/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_OS_TRANSMITTER_H__
#define __BLE_OS_TRANSMITTER_H__

#include "data_type/DataTypes.h"


/**
 * 
 * \return 0 to allow the connection
 */
typedef int (*BleOsTransmitter_allow_connection_cb)(void * module, const BleAddress_t * addr);

typedef int (*BleOsTransmitter_get_payload_cb)(void * module, const BleAddress_t * addr,
    Data_t * payload);

/**
 * \brief The callback when another device has written to our write characteristic
 * 
 * For now this callback should be called when a 
 * device disconnects, the offset should be negative,
 * If the device has not written anything then nothing will happen
 * 
 * \param module
 * \param addr
 * \param data
 * \param offset the offset, set to negative to inidate the end of the payload
 * 
 * \return 0 for success, negative size was to big
 */
typedef int (*BleOsTransmitter_received_payload_cb)(void * module, const BleAddress_t * addr, Data_t * data, int offset);

int BleOsTransmitter_init(void * module,
    BleOsTransmitter_allow_connection_cb allow_connection_cb,
    BleOsTransmitter_get_payload_cb get_payload_cb,
    BleOsTransmitter_received_payload_cb received_payload_cb);

#endif /* __BLE_OS_TRANSMITTER_H__ */