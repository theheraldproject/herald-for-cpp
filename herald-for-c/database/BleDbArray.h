/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_DB_ARRAY_H__
#define __BLE_DB_ARRAY_H__

#include <stdint.h>
#include "data_type/DataTypes.h"
#include "ble/BleDevice.h"

#include <zephyr.h>

#define BleDbArray_DEF() {0}

typedef struct BleDbArray_entry_s
{
    uint8_t used;
    /**<
     * Marks if this data is currently used
     * 0 for NOT used, not zero if used
     */
    BleAddress_t addr;
    /**< The device address */
    BleDevice_t dev;
    /**< The BLE device stored */
}
BleDbArray_entry_t;

typedef struct ble_db_array_s
{
    size_t size;
    /**< The total number of devices used,
     * only stored for easy access to the size,
     * the algoriths do not depend on this */
    BleDbArray_entry_t entryPool[CONFIG_HERALD_MAX_DEVS_IN_DB];
    /**< The memory pool of entries */
    struct k_mutex mutex;
    /**< Database mutex */
}
BleDbArray_t;

/* Define the type of DB data structure as a Linked List */
typedef BleDbArray_t BleDbDataStruct_t;

/* Make the static definition */
#define BleDbDataStruct_DEF() BleDbArray_DEF()

#endif /* __BLE_DB_ARRAY_H__ */