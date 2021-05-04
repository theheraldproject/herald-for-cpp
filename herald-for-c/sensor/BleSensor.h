/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_SENSOR_H__
#define __BLE_SENSOR_H__

#include "ble/BleScanner.h"
#include "ble/BleAdvertiser.h"
#include "ble/BleReader.h"
#include "ble/BleTransmitter.h"

#include "database/BleDatabase.h"

#define BleSensor_database_INIT(_p_sensor_delegate) \
    BleDatabase_DEF( \
        DatabaseDelegate_DEF( \
            _p_sensor_delegate, \
            BleSensor_db_didCreate, \
            BleSensor_db_didUpdate, \
            BleSensor_db_didDelete \
        ) \
    )

#define BleSensor_DEF(_ble_database, _p_scan_queue, _p_payload_queue) \
{ \
    BleScanner_DEF(), \
    BleAdvertiser_DEF(), \
    BleReader_DEF(), \
    BleTransmitter_DEF(), \
    _ble_database, \
    _p_scan_queue, \
    _p_payload_queue \
}

typedef struct ble_sensor_s
{
    BleScanner_t scanner;
    BleAdvertiser_t advertiser;
    BleReader_t reader;
    BleTransmitter_t transmitter;
    BleDatabase_t * database;
    struct k_msgq * scan_queue;
    struct k_msgq * payload_queue;
}
BleSensor_t;

int BleSensor_init(BleSensor_t * self);

int BleSensor_start(BleSensor_t * self);

void BleSensor_db_didCreate(void * module, const BleAddress_t * addr);

void BleSensor_db_didUpdate(void * module, const BleAddress_t * addr,  
    BleDeviceAttribute_t attribute, void * data);

void BleSensor_db_didDelete(void * module, const BleAddress_t * addr);

void BleSensor_process_scan(BleSensor_t * self);
void BleSensor_process_payload(BleSensor_t * self);
void BleSensor_update_payload(BleSensor_t * self, Data_t * payload);


#endif /* __BLE_SENSOR_H__ */