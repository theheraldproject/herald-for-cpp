/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_ADVERTISER_H__
#define __BLE_ADVERTISER_H__

#include "ble/BleOsAdvertiser.h"

#define BleAdvertiser_DEF() \
{ \
    \
}

typedef struct ble_advertiser_s
{
    int none;
}
BleAdvertiser_t;

static inline int BleAdvertiser_init(BleAdvertiser_t * self)
{
    return BleOsAdvertiser_init();
}

static inline int BleAdvertiser_start(BleAdvertiser_t * self)
{
    return BleOsAdvertiser_start();
}

#endif /* __BLE_ADVERTISER_H__ */