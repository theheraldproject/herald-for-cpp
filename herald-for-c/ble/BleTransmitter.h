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

#define CONFIG_HERRALD_MAX_TX_RECEIVE_AT_ONE_TIME 3

struct ble_tx_incoming_messages
{
    uint8_t used; /**< == 0 for unused, != 0 for used */
    BleAddress_t addr;
    uint8_t payload[CONFIG_HERALD_MAX_PAYLOAD_SIZE];
    uint16_t current_payload_sz;
};

typedef struct ble_transmitter_s
{
    uint8_t payload[CONFIG_HERALD_MAX_PAYLOAD_SIZE];
    uint16_t payload_size;
    struct k_mutex payload_mutex;
    struct ble_tx_incoming_messages incoming_msgs[CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE];
    struct k_msgq * payload_queue;
}
BleTransmitter_t;

int BleTransmitter_init(BleTransmitter_t * self, struct k_msgq * payload_queue);
int BleTransmitter_set_payload(BleTransmitter_t * self, Data_t * payload);
int BleTransmitter_start(BleTransmitter_t * self);
int BleTransmitter_stop(BleTransmitter_t * self);

#endif /* __BLE_TRANSMITTER_H__ */