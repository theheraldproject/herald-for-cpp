/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_ERR_CODES__
#define __BLE_ERR_CODES__

/* Command to stop reading */
#define BleErr_STOP_READING                      1
/* No error */
#define BleErr_OK                                0
/* System error, starting a process */
#define BleErr_SYSTEM                           -1
/* Could not connect error */ 
#define BleErr_ERR_CONNECTING                   -2
/* Error while doing GATT discovery */
#define BleErr_ERR_GATT_DISCOVERY               -3
/* Herald service was not found */
#define BleErr_ERR_HERALD_SERVICE_NOT_FOUND     -4
/* Herald payload characteristic was not found */
#define BleErr_ERR_HERALD_PAYLOAD_NOT_FOUND     -5
/* Payload too big to fit in buffer */
#define BleErr_ERR_PAYLOAD_TO_BIG               -6

#endif /* __BLE_ERR_CODES__ */