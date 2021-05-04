/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include "ble/BleOsScanner.h"
#include "ble/zephyr/zephyr_ble.h"

static void * prv_cb_module = NULL;
static BleOsScanner_cb_t prv_cb = NULL;


void BleZephyrScanner_cb(const bt_addr_le_t *addr, int8_t rssi, Data_t * manufacturer_data)
{
    assert(prv_cb);

    BleAddress_t hrldAddr;

    hrld_addr_from_zephyr_addr(&hrldAddr, addr);

    prv_cb(prv_cb_module, &hrldAddr, manufacturer_data, rssi);
}

int BleOsScanner_init(BleOsScanner_cb_t scan_cb, void * module)
{
    prv_cb = scan_cb;
    prv_cb_module = module;
    
    return zephyr_scan_init();
}


static uint8_t prv_req_stop = 0;
static uint8_t prv_should_be_on = 0;

K_MUTEX_DEFINE(prv_scan_mutex);

static inline void prv_lock(void)
{
    k_mutex_lock(&prv_scan_mutex, K_FOREVER);
}

static inline void prv_unlock(void)
{
    k_mutex_unlock(&prv_scan_mutex);
}

int BleOsScanner_start(void)
{
    int err;
    prv_lock();
    prv_should_be_on = 1;
    err = zephyr_scan_start();
    //TODO: check prv_scan_req_stop
    prv_unlock();
    return err;
}

int BleOsScanner_stop(void)
{
    int err;
    prv_lock();
    prv_should_be_on = 0;
    err = zephyr_scan_stop();
    prv_unlock();
	return err;
}

void BleZephyrScan_allow(void)
{
    prv_lock();
    /* Make sure count is not already at zero */
    if(prv_req_stop == 0)
    {
        LOG_ERR("Request scan stop!");
        prv_unlock();
        return;
    }

    /* Decrement */
    prv_req_stop--;

    if(prv_req_stop == 0 && prv_should_be_on != 0)
    {
        /* Resart the scanner */
        zephyr_scan_start();
        LOG_DBG("Scanner started");
        k_sleep(K_MSEC(100));
    }
    prv_unlock();
}

void BleZephyrScan_disallow(void)
{
    prv_lock();
    /* Check if it's the first request, and scanner should be on
    If this is the case then the scanner IS on */
    if(prv_req_stop == 0 && prv_should_be_on != 0)
    {
        /* Stop the scanner */
        zephyr_scan_stop();
        LOG_DBG("Scanner stopped");
    }

    /* Increment */
    prv_req_stop++;
    prv_unlock();
}