/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __DATABASE_DELEGATE__
#define __DATABASE_DELEGATE__

#include "ble/BleDevice.h"

#define DatabaseDelegate_DEF(_pv_module, _didCreateCb, _didUpdateCb, _didDeleteCb) \
{ \
    (_pv_module), (_didCreateCb), (_didUpdateCb), (_didDeleteCb) \
}

/**
 * The didCreate callback, called everytime a new device is created
 */
typedef void (*DatabaseDelegate_didCreate_t)(void * module, const BleAddress_t * pseudo);

/**
 * The didUpdate callback, called everytime an attribute of the device is updated
 */
typedef void (*DatabaseDelegate_didUpdate_t)(void * module, const BleAddress_t * pseudo, 
    BleDeviceAttribute_t attrubute, void * data);

/**
 * The didDelete callback, called everytime a device is removed from the database
 */
typedef void (*DatabaseDelegate_didDelete_t)(void * module, const BleAddress_t * pseudo);

/**
 * The database delegate
 */
typedef struct database_delgate_s
{
    void * module;
    DatabaseDelegate_didCreate_t didCreateCb;
    DatabaseDelegate_didUpdate_t didUpdateCb;
    DatabaseDelegate_didDelete_t didDeleteCb;
}
DatabaseDelegate_t;

static inline void DatabaseDelegate_didCreate(const DatabaseDelegate_t * self,
    const BleAddress_t * pseudo)
{
    if(self->didCreateCb != NULL)
    {
        self->didCreateCb(self->module, pseudo);
    }
}

static inline void DatabaseDelegate_didUpdate(const DatabaseDelegate_t * self, const BleAddress_t * pseudo,
    BleDeviceAttribute_t attribute, void * data)
{
    if(self->didUpdateCb != NULL)
    {
        self->didUpdateCb(self->module, pseudo, attribute, data);
    }
}

static inline void DatabaseDelegate_didDelete(const DatabaseDelegate_t * self, const BleAddress_t * pseudo)
{
    if(self->didDeleteCb != NULL)
    {
        self->didDeleteCb(self->module, pseudo);
    }
}



#endif /* __DATABASE_DELEGATE__ */