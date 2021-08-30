/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "sensor/SensorDelegate.h"
#include "ble/BleDevice.h"
#include "logger/herald_logger.h"

void BleSensor_db_didCreate(void * module, const BleAddress_t * pseudo)
{
    /* Run the didDetect callback */
    SensorDelegate_didDetect((SensorDelegate_t*) module, pseudo);
}

void BleSensor_db_didUpdate(void * module, const BleAddress_t * pseudo,  
    BleDeviceAttribute_t attribute, void * data)
{
    Rssi_t rssi;

    assert(module);

    SensorDelegate_t * sensor_delegate = (SensorDelegate_t*) module;

    switch(attribute)
    {
        case BleDeviceAttrRSSI:
            rssi = *((Rssi_t*) data);
            SensorDelegate_didMeasure(sensor_delegate, pseudo, (double) rssi);
            break;

        case BleDeviceAttrPAYLOAD_DATA:
            SensorDelegate_didRead(sensor_delegate, pseudo, (Data_t *) data);
            break;

        default:
            /* Not logging anything else for now */
            break;
    }
}

void BleSensor_db_didDelete(void * module, const BleAddress_t * pseudo)
{
    LOG_DBG(BleAddr_printStr() " DidDelete ", BleAddr_printParams(pseudo));
    /* Currently only logging is needed */
}