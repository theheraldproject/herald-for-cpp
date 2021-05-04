/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_READER_H__
#define __BLE_READER_H__

#include <zephyr.h>

#define BleReader_DEF() {}

struct payload_msg
{
    int8_t status;
    BleAddress_t pseudo;
    uint8_t payload[CONFIG_HERALD_MAX_PAYLOAD_SIZE];
    size_t payload_sz;
}__attribute__((aligned(4)));

struct ble_payload_readings_s
{
    uint8_t used; /**< == 0 for unused, != 0 for used */
    BleAddress_t addr;
    BleAddress_t pseudo;
    uint8_t payload[CONFIG_HERALD_MAX_PAYLOAD_SIZE];
    uint16_t current_payload_sz;
};

typedef struct ble_reader_s
{
    struct k_msgq * payload_queue;
    struct k_sem conn_sem;
    struct ble_payload_readings_s payload_readings[CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME];
}
BleReader_t;

int BleReader_init(BleReader_t * self, struct k_msgq * payload_queue);

int BleReader_read_payload(BleReader_t * self, const BleAddress_t * addr,
    const BleAddress_t * pseudo);

#endif /* __BLE_READER_H__ */