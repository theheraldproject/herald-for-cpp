/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_DATABASE__
#define __BLE_DATABASE__

#include "database/DatabaseDelegate.h"
#include "database/BleDbDataStruct.h"
#include "ble/BleDevice.h"
#include "sys/timestamp.h"

#include "logger/herald_logger.h"

#define BleDatabase_DEF(_database_delegate) \
{ \
    _database_delegate, \
    BleDbDataStruct_DEF(), \
    {} \
}

/**
 * A database to store results in
 */
typedef struct ble_database_s
{
    const DatabaseDelegate_t delegate;
    /**< The database delegate to call when updating the database */

    BleDbDataStruct_t deviceList;
    /**< The devices that exist in the database */

    struct k_mutex mutex;
    /**< Mutex to for the database and devices */
}
BleDatabase_t;

static inline void BleDatabase_init(BleDatabase_t * self)
{
    BleDbDataStruct_init(&self->deviceList);
    k_mutex_init(&self->mutex);
}

static inline void BleDatabase_lock(BleDatabase_t * self)
{
    k_mutex_lock(&self->mutex, K_FOREVER);
}

static inline void BleDatabase_unlock(BleDatabase_t * self)
{
    k_mutex_unlock(&self->mutex);
}

BleDevice_t * BleDatabase_find_create_device(BleDatabase_t * self, const BleAddress_t * addr);

static inline void BleDatabase_herald_not_found(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDatabase_lock(self);
    BleDevice_herald_not_found(dev);
    BleDatabase_unlock(self);
}

static inline void BleDatabase_connection_error(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDatabase_lock(self);
    BleDevice_connection_error(dev);
    BleDatabase_unlock(self);
}

static inline void BleDatabase_set_state(BleDatabase_t * self, BleDevice_t * dev, BleDevice_state_t state)
{
    BleDatabase_lock(self);
    BleDevice_set_state(dev, state);
    BleDatabase_unlock(self);
}

static inline void BleDatabase_scanned(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDatabase_lock(self);
    BleDevice_scanned(dev);
    BleDatabase_unlock(self);
}

static inline int BleDatabase_should_connect(BleDatabase_t * self, BleDevice_t * dev)
{
    int ret;
    BleDatabase_lock(self);
    ret = BleDevice_shouldConnect(dev);
    BleDatabase_unlock(self);
    return ret;
}

static inline BleDevice_state_t BleDatabase_get_state(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDevice_state_t ret;
    BleDatabase_lock(self);
    ret = BleDevice_get_state(dev);
    BleDatabase_unlock(self);
    return ret;
}

static inline int BleDatabase_payload_needs_read(BleDatabase_t * self, BleDevice_t * dev)
{
    int ret;
    BleDatabase_lock(self);
    ret = BleDevice_payloadNeedsRead(dev);
    BleDatabase_unlock(self);
    return ret;
}

/**
 * \brief Log scan of the device and update the most recent RSSI in the database
 * 
 * \param self self
 * \param addr The BleAddress of the device
 * \param dev The device, can be NULL
 * \param rssi The RSSI of the most recent call
 *
 */
static inline void BleDatabase_record_rssi(BleDatabase_t * self,
    const BleAddress_t * addr, BleDevice_t * dev, Rssi_t rssi)
{
    assert(self);

    if(dev != NULL)
    {
        /* Nothing to do yet, NULL device is allowes here */
    }

    /* Call the didUpdate callback */
    DatabaseDelegate_didUpdate(&self->delegate, addr, BleDeviceAttrRSSI, (void*) &rssi);
}

static inline void BleDatabase_read_payload(BleDatabase_t * self,
    const BleAddress_t * addr, BleDevice_t * dev, const Data_t * data)
{
    assert(self);

    /* Set the last payload read time */
    BleDevice_payload_read(dev);

    DatabaseDelegate_didUpdate(&self->delegate, addr, BleDeviceAttrPAYLOAD_DATA, (void*) data);
}

void BleDatabase_remove_old_devices(BleDatabase_t * self);

#endif /* __BLE_DATABASE__ */