/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_OS_ADVERTISER_H__
#define __BLE_OS_ADVERTISER_H__


/**
 * \brief Initialize the advertiser
 * 
 * 
 * \return 0 for success, error code otherwise
 */
int BleOsAdvertiser_init(void);

/**
 * \brief Start advertising
 * 
 * \return 0 for success, error code otherwise 
 */
int BleOsAdvertiser_start(void);

/**
 * @brief Stop advertising
 * 
 * \return 0 for success, error code otherwise 
 */
int BleOsAdvertiser_stop(void);

#endif /* __BLE_OS_ADVERTISER_H__ */