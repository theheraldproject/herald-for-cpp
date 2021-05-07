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

/**
 * \defgroup BleDatabase Ble Database
 * The Ble database is responsible for adding/removing devices
 * mutual exclusion of BleDevice call and the database
 * Calling DB delegates
 * It is not responsible for any logic on the devices
 * \{
 */

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

/**
 * \brief Notify a read is starting 
 * 
 * \param self 
 * \param dev 
 */
static inline void BleDatabase_payload_start_reading(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDatabase_lock(self);
    BleDevice_startingRead(dev);
    BleDatabase_unlock(self);
}

/**
 * \brief Notify a payload read failed
 * 
 * \param self 
 * \param dev 
 * \param err reason for the fail 
 */
static inline void BleDatabase_payload_not_read(BleDatabase_t * self, BleDevice_t * dev, int8_t err)
{
    BleDatabase_lock(self);
    BleDevice_payload_not_read(dev, err);
    BleDatabase_unlock(self);
}

/**
 * \brief Notify a payload read was successfull and call the delegate
 * 
 * \param self 
 * \param addr 
 * \param dev 
 * \param data 
 */
static inline void BleDatabase_read_payload(BleDatabase_t * self,
    const BleAddress_t * addr, BleDevice_t * dev, const Data_t * data)
{
    assert(self);

    /* Recored the payload read */
    BleDevice_payload_read_success(dev);

    /* Call the delegate */
    DatabaseDelegate_didUpdate(&self->delegate, addr, BleDeviceAttrPAYLOAD_DATA, (void*) data);
}

/**
 * \brief Notify scan of device 
 * 
 * \param self 
 * \param dev 
 */
static inline void BleDatabase_scanned(BleDatabase_t * self, BleDevice_t * dev)
{
    BleDatabase_lock(self);
    BleDevice_scanned(dev);
    BleDatabase_unlock(self);
}

/**
 * \brief Check if the device needs a payload read
 * 
 * \param self 
 * \param dev 
 * \return ==0 it does not need to be read
 * \return !=0 it should be read 
 */
static inline int BleDatabase_payload_should_read(BleDatabase_t * self, BleDevice_t * dev)
{
    int ret;
    BleDatabase_lock(self);
    ret = BleDevice_payloadShouldRead(dev);
    BleDatabase_unlock(self);
    return ret;
}

/**
 * \brief Just call the delegate
 * 
 * \param self self
 * \param addr The BleAddress of the device
 * \param dev The device, can be NULL
 * \param rssi The RSSI of the most recent call
 */
static inline void BleDatabase_rssi_found(BleDatabase_t * self,
    const BleAddress_t * addr, Rssi_t rssi)
{
    assert(self);

    /* Call the didUpdate callback */
    DatabaseDelegate_didUpdate(&self->delegate, addr, BleDeviceAttrRSSI, (void*) &rssi);
}

void BleDatabase_remove_old_devices(BleDatabase_t * self);

/** \} */

#endif /* __BLE_DATABASE__ */