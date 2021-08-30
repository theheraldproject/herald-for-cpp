/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_OS_READER_H__
#define __BLE_OS_READER_H__

#include "data_type/DataTypes.h"


/**
 * \brief The payload read callback
 * 
 * If status is non zero the same status will be returned
 * Otherwise non zero will be returned in case the payload should not be read any more
 * Zero will be returned if more data should be read 
 */
typedef int (*BleOsReader_payload_cb_t)(void * module, const BleAddress_t * addr, int status, const Data_t * data);


/**
 * \brief Signal the connection is done and a new one can start
 * 
 */
typedef void (*BleOsReader_connection_done_cb_t)(void * module, const BleAddress_t * addr);

/**
 * \brief Initialize the payload reader
 * 
 * \param payload_cb The callback to be called on read or error
 * \param module Any user data to pass the callback
 * 
 * \return 0 for success, error otherwise
 */
int BleOsReader_init(void * module, BleOsReader_payload_cb_t payload_cb,
    BleOsReader_connection_done_cb_t con_done_cb);

/**
 * \brief Start the read of a payload from a specified address
 * 
 * If this returns success then the
 * payload callback WILL be called. Either upon error or success
 * 
 * \return 0 for success, error code otherwise
 */
int BleOsReader_get_payload(const BleAddress_t * addr);


#endif /* __BLE_OS_READER_H__ */