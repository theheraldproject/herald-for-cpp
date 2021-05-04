/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_OS_SCANNER_H__
#define __BLE_OS_SCANNER_H__

#include "data_type/DataTypes.h"

typedef void (*BleOsScanner_cb_t)(void * module, const BleAddress_t * macAddr,
    Data_t * manufacturer_data, Rssi_t rssi);

/**
 * \brief Initialize the scanner
 * 
 * \param scan_cb Callback everytime a device passes through the filter
 * \param module A parameter that is passed back with the scan_cb
 * 
 * \return 0 for success
 */
int BleOsScanner_init(BleOsScanner_cb_t scan_cb, void * module);

/**
 * \brief Start scanning for peripherals
 * the scan callback will be called as devices are found
 * 
 * \return 0 for success 
 */
int BleOsScanner_start(void);

/**
 * \brief Stop scanning
 * 
 * \return 0 for success 
 */
int BleOsScanner_stop(void);

#endif /* __BLE_OS_SCANNER_H__ */