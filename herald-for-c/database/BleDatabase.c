/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "database/BleDatabase.h"

/**
 * \brief Create a new device and add it to the database
 * 
 * \param self 
 * \param addr 
 * \return Pointer to the device in the database, NULL if it could not be created 
 */
static BleDevice_t * prv_create_device(BleDatabase_t * self, const BleAddress_t * addr)
{
    assert(self);

    /* Create the new device */
    BleDevice_t * dev = BleDbDataStruct_add_entry(&self->deviceList, addr);

    /* Check success */
    if(dev == NULL)
    {
        LOG_ERR("Error creating device");
        return NULL;
    }

    /* Initialize the data structure */
    BleDevice_init(dev);

    /* Success, call the didCreate callback */
    DatabaseDelegate_didCreate(&self->delegate, addr);

    return dev;
}

/**
 * \brief Find or create a device with the given address
 * 
 * If a device with the address already exists, return it
 * Otherwise, create a new one and add it to the database
 * 
 * In the future this should be made more efficient by searching the ordered list
 * and if not found, instert it right there, currently we are doing two passes
 * 
 * @param self 
 * @param addr 
 * @return A pointer to the device in the database,
 * NULL if not found and not enough memory to create a new one
 */
BleDevice_t * BleDatabase_find_create_device(BleDatabase_t * self, const BleAddress_t * addr)
{
    BleDevice_t * dev;

    assert(self);

    /* Attempt to find it in the database */
    dev = BleDbDataStruct_find(&self->deviceList, addr);

    /* If not found then create a new one */
    if(dev == NULL)
    {
        dev = prv_create_device(self, addr);
    }

    return dev;
}

static int prv_db_delete_old_dev_cb(const BleAddress_t * addr, BleDevice_t * dev,
    void * param1, void * param2)
{
    uint64_t dev_ms;

    BleDatabase_t * self = (BleDatabase_t*) param1;

    uint64_t * curr_ms = (uint64_t*) param2;

    /* Get the last scanned time in MS */
    Timestamp_get_ms(&dev_ms, &dev->lastScan);

    /* Check if the device should be delete */
    if((*curr_ms - dev_ms)/1000 > CONFIG_HERALD_DEVICE_EXPIRY_SEC)
    {
        /* Call did delete callback */
        DatabaseDelegate_didDelete(&self->delegate, addr);
        /* tell DB to delete it */
        return 1;
    }

    return 0;
}



void BleDatabase_remove_old_devices(BleDatabase_t * self)
{
    assert(self);

    uint64_t now_ms;

    Timestamp_now_ms(&now_ms);


    BleDbDataStruct_loop_devs(&self->deviceList, prv_db_delete_old_dev_cb, 
        (void*) self, (void*) &now_ms);
}