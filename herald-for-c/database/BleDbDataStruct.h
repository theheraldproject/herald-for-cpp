/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_DB_DATA_STRUCT__
#define __BLE_DB_DATA_STRUCT__

#include "data_type/DataTypes.h"
#include "ble/BleDevice.h"
#include "database/BleDbArray.h"

/**
 * \brief callback when looping through device list
 * \param dev The device 
 * \param param The user defined parameter
 * 
 * \return 0 for the device to be kept, not 0 for it to be removed
 */
typedef int (*BleDbDataStruct_loop_cb_t)(const BleAddress_t * addr,  BleDevice_t * dev,
    void * param1, void * param2);

/**
 * \brief Initialize the DB struct 
 * 
 * \param self 
 */
void BleDbDataStruct_init(BleDbArray_t * self);

/**
 * \brief Allocates memory for the device at the given address
 * 
 * \param self 
 * \param address 
 * \return The device memory, NULL if it was not successfull
 */
BleDevice_t * BleDbDataStruct_add_entry(BleDbDataStruct_t * self, const BleAddress_t * address);

/**
 * \brief Get a device from the DB at the given address 
 * 
 * \param self 
 * \param addr 
 * \return Pointer to the found device, NULL if not found 
 */
BleDevice_t * BleDbDataStruct_find(BleDbDataStruct_t * self, const BleAddress_t * addr);

/**
 * \brief Get the number of devices in the DB
 * 
 * \param self 
 * \return The number of devices 
 */
size_t BleDbDataStruct_get_size(BleDbDataStruct_t * self);

/**
 * \brief Loops through all devices and performs the callback
 * If the callback return 0 the device is not deleted, otherwise it is 
 * 
 * @param self 
 * @param param A parameter to pass to the callback
 */
void BleDbDataStruct_loop_devs(BleDbDataStruct_t * self, BleDbDataStruct_loop_cb_t cb,
    void * param1, void * param2);


#endif /* __BLE_DB_DATA_STRUCT__ */