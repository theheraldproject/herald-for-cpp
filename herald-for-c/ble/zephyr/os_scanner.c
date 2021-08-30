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

void start_scanner_work_handler(struct k_work * work)
{
    zephyr_scan_start();
    LOG_DBG("Scanner started");
}

K_WORK_DEFINE(start_scanner_work, start_scanner_work_handler);

void scanner_work_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&start_scanner_work);
}

K_TIMER_DEFINE(scanner_work_timer, scanner_work_timer_handler, NULL);

//#define prvSCAN_START_DELAY (CONFIG_HERALD_SCAN_INTERVAL_MS - CONFIG_HERALD_SCAN_WINDOW_MS)
#define prvSCAN_START_DELAY_MS (200)

static void prv_start_scanner(void)
{
    k_timer_start(&scanner_work_timer, K_MSEC(prvSCAN_START_DELAY_MS), K_NO_WAIT);
}

static void prv_stop_scanner(void)
{
    if(k_timer_status_get(&scanner_work_timer) == 0)
    {
        LOG_DBG("Stopping timmer..");
    }

    /* Stop no matter what */
    k_timer_stop(&scanner_work_timer);
    zephyr_scan_stop();
}

void BleZephyrScanner_cb(const bt_addr_le_t *addr, int8_t rssi,
    Data_t * manufacturer_data, uint8_t status)
{
    assert(prv_cb);

    BleAddress_t hrldAddr;

    hrld_addr_from_zephyr_addr(&hrldAddr, addr);

    prv_cb(prv_cb_module, &hrldAddr, manufacturer_data, rssi, status);
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
        prv_start_scanner();
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
        prv_stop_scanner();
    }

    /* Increment */
    prv_req_stop++;
    prv_unlock();
}