/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_SCANNER_H__
#define __BLE_SCANNER_H__

#include "data_type/DataTypes.h"

#define BleScanner_DEF() \
{ \
    NULL \
}

struct scan_results_message
{
    BleAddress_t addr;
    /**< MAC address */
    BleAddress_t pseudo;
    /**< Pseudo address */
    Rssi_t rssi;
    /**< RSSI */
    uint8_t could_be_herald;
    /**< 0 if it is not required to connect
     * This is used so we have the option to disable the filter
     * so everydevice can have a didMeasure callback
     */
}
__attribute__((aligned(4)));

typedef struct ble_scanner_s
{
    struct k_msgq * scan_res_queue;
}
BleScanner_t;

int BleScanner_init(BleScanner_t * self, struct k_msgq * scan_res_queue);

int BleScanner_start(BleScanner_t * self);

#endif /* __BLE_SCANNER_H__ */