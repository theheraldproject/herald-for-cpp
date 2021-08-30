/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/BleOsAdvertiser.h"
#include "ble/zephyr/zephyr_ble.h"

static uint8_t prv_stop_req = 0;
static uint8_t prv_should_be_on = 0;

K_MUTEX_DEFINE(prv_advert_mutex);

static inline void prv_lock(void)
{
    k_mutex_lock(&prv_advert_mutex, K_FOREVER);
}

static inline void prv_unlock(void)
{
    k_mutex_unlock(&prv_advert_mutex);
}


int BleOsAdvertiser_init(void)
{
    return 0;
}

int BleOsAdvertiser_start(void)
{
    int err;
    prv_lock();
    prv_should_be_on = 1;
    err = zephyr_advertise_start();
    prv_unlock();
    return err;
}

int BleOsAdvertiser_stop(void)
{
    int err;
    prv_lock();
    prv_should_be_on = 0;
    err = zephyr_advertise_stop();
    prv_unlock();
    return err;
}

void BleZephyrAdvertise_allow(void)
{
    prv_lock();
    /* Make sure count is not already at zero */
    if(prv_stop_req == 0)
    {
        LOG_ERR("Request scan stop!");
        prv_unlock();
        return;
    }

    /* Decrement */
    prv_stop_req--;

    if(prv_stop_req == 0 && prv_should_be_on != 0)
    {
        /* Resart the advertiser */
        zephyr_advertise_start();
        LOG_DBG("Advertising started");
    }
    prv_unlock();
}

void BleZephyrAdvertise_disallow(void)
{
    prv_lock();
    /* Check if it's the first request, and scanner should be on
    If this is the case then the scanner IS on */
    if(prv_stop_req == 0 && prv_should_be_on != 0)
    {
        zephyr_advertise_stop();
        LOG_DBG("Advertising stopped");
    }

    /* Increment */
    prv_stop_req++;
    prv_unlock();
}