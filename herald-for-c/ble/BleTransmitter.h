/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_TRANSMITTER_H__
#define __BLE_TRANSMITTER_H__

#include "data_type/DataTypes.h"
#include <zephyr.h>

#define BleTransmitter_DEF() {{},0}



typedef struct ble_transmitter_s
{
    uint8_t payload[CONFIG_HERALD_MAX_PAYLOAD_SIZE];
    uint16_t payload_size;
    struct k_mutex payload_mutex;
}
BleTransmitter_t;

int BleTransmitter_init(BleTransmitter_t * self);
int BleTransmitter_set_payload(BleTransmitter_t * self, Data_t * payload);
int BleTransmitter_start(BleTransmitter_t * self);
int BleTransmitter_stop(BleTransmitter_t * self);

#endif /* __BLE_TRANSMITTER_H__ */