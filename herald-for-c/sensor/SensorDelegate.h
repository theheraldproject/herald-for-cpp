/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __SENSOR_DELEGATE__
#define __SENSOR_DELEGATE__

#include "data_type/DataTypes.h"
#include "sensor/Sensor.h"

#define SensorDelegate_INIT( \
    _didDetect, _didRead, _didReceive, _didShare, \
    _didMeasure, _didVisit, _didUpdate) \
{ \
    (_didDetect),(_didRead),(_didReceive), (_didShare), \
    (_didMeasure), (_didVisit), (_didUpdate) \
}

typedef void (*SensorDelegate_didDetect_t)(const BleAddress_t * pseudo);

typedef void (*SensorDelegate_didMeasure_t)(const BleAddress_t * pseudo, double rssi);

typedef void (*SensorDelegate_didRead_t)(const BleAddress_t * pseudo, Data_t * paylaodData);

typedef void (*SensorDelegate_didReceive_t)(const BleAddress_t * pseudo, Data_t * immediateSendData);

typedef void (*SensorDelegate_didShare_t)(const BleAddress_t * pseudo, Data_t immediateSendData[], size_t dataLen);

typedef void (*SensorDelegate_didVisit_t)(int location);

typedef void (*SensorDelegate_didUpdateState_t)(SensorState_t sensorState);

typedef struct sensor_delegate_s
{
    SensorDelegate_didDetect_t didDetect;
    SensorDelegate_didRead_t didRead;
    SensorDelegate_didReceive_t didReceive;
    SensorDelegate_didShare_t didShare;
    SensorDelegate_didMeasure_t didMeasure;
    SensorDelegate_didVisit_t didVisit;
    SensorDelegate_didUpdateState_t didUpdateState;
}
SensorDelegate_t;

static inline void SensorDelegate_didDetect(SensorDelegate_t * self, const BleAddress_t * pseudo)
{
    assert(self);

    if(self->didDetect != NULL)
    {
        self->didDetect(pseudo);
    }
}

static inline void SensorDelegate_didMeasure(SensorDelegate_t * self,
    const BleAddress_t * pseudo, double rssi)
{
    assert(self);

    if(self->didMeasure != NULL)
    {
        self->didMeasure(pseudo, rssi);
    }
}

static inline void SensorDelegate_didRead(SensorDelegate_t * self,
    const BleAddress_t * pseudo, Data_t * payload)
{
    assert(self);

    if(self->didRead != NULL)
    {
        self->didRead(pseudo, payload);
    }
}

#endif /* __SENSOR_DELEGATE__ */