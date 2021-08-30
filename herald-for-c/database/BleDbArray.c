/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "database/BleDbDataStruct.h"


void BleDbDataStruct_init(BleDbArray_t * self)
{
    /* Wipe the whole thing */
    memset(self->entryPool, 0, sizeof(BleDbArray_entry_t) * CONFIG_HERALD_MAX_DEVS_IN_DB);
    /* Initialize the mutex */
    k_mutex_init(&self->mutex);
}

static inline int prv_lock(BleDbArray_t * self)
{
    return k_mutex_lock(&self->mutex, K_FOREVER);
}

static inline void prv_unlock(BleDbArray_t * self)
{
    k_mutex_unlock(&self->mutex);
}

BleDevice_t * BleDbDataStruct_add_entry(BleDbArray_t * self, const BleAddress_t * addr)
{
    size_t i;

    if(prv_lock(self) != 0) return NULL;

    /* Find next available device */
    for(i=0;i<CONFIG_HERALD_MAX_DEVS_IN_DB;i++)
    {
        /* Only look for NOT used entries */
        if(self->entryPool[i].used != 0)
        {
            continue;
        }

        /* Set to used */
        self->entryPool[i].used = 1;

        /* Copy the address */
        BleAddress_copy(&self->entryPool[i].addr, addr);

        /* Increment the size counter */
        self->size++;

        prv_unlock(self);
        return &self->entryPool[i].dev;
    }
    prv_unlock(self);
    return NULL;
}

BleDevice_t * BleDbDataStruct_find(BleDbArray_t * self, const BleAddress_t * addr)
{
    size_t i;

    if(prv_lock(self) != 0) return NULL;

    for(i=0;i<CONFIG_HERALD_MAX_DEVS_IN_DB;i++)
    {
        /* Only look for used entries */
        if(self->entryPool[i].used == 0)
        {
            continue;
        }

        /* Compare the addresses */
        if(BleAddress_cmp(&self->entryPool[i].addr, addr) != 0)
        {
            continue;
        }

        /* It was found */
        prv_unlock(self);
        return &self->entryPool[i].dev;
    }
    prv_unlock(self);
    return NULL;
}

size_t BleDbDataStruct_get_size(BleDbArray_t * self)
{
    if(prv_lock(self) != 0) return 0;
    return self->size;
    prv_unlock(self);
}

void BleDbDataStruct_loop_devs(BleDbArray_t * self, BleDbDataStruct_loop_cb_t cb,
    void * param1, void * param2)
{
    size_t i;
    int ret;

    assert(self);
    assert(cb);

    if(prv_lock(self) != 0) return;

    for(i=0;i<CONFIG_HERALD_MAX_DEVS_IN_DB; i++)
    {
        /* Only interested in devices that are in use */
        if(self->entryPool[i].used == 0)
        {
            continue;
        }

        /* Call the callback */
        ret = cb(&self->entryPool[i].addr, &self->entryPool[i].dev, param1, param2);

        if(ret != 0)
        {
            /* Delete the device */
            self->entryPool[i].used = 0;
            self->size--;
        }
    }
    prv_unlock(self);
}
