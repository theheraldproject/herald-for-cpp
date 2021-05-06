/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/zephyr/zephyr_ble.h"

#define CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE 3

#define prvTBL_SZ CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE
#define prvMAX_CON_TIME_MS CONFIG_HERALD_MAX_INCOMING_CONNECTION_TIME_MS

struct in_con_s
{
    struct bt_conn * conn;
};

static struct in_con_s prv_db[prvTBL_SZ];

void zephyr_in_cons_init(void)
{
    /* Zero the db */
    memset(prv_db, 0, sizeof(struct in_con_s) * prvTBL_SZ);
}

void in_con_disconnect_work_handler(struct k_work * work)
{
    size_t i;
    for(i=0;i<prvTBL_SZ;i++)
    {
        if(prv_db[i].conn != NULL)
        {
            LOG_WRN("Forcing disconnection!");
            /* Disconnect, the actual disconnection event will remove it from this table */
            zephyr_connection_disconnect(prv_db[i].conn);
        }
    }
}

K_WORK_DEFINE(in_con_disconnect_work, in_con_disconnect_work_handler);


static void in_con_disconnect_timer_handler()
{
    k_work_submit(&in_con_disconnect_work);
}

K_TIMER_DEFINE(in_con_disconnect_timer, in_con_disconnect_timer_handler, NULL);



static void prv_restart_disconnect_timer(void)
{
    /* Start the timer */
    k_timer_start(&in_con_disconnect_timer, 
        K_MSEC(prvMAX_CON_TIME_MS), K_NO_WAIT);
}

static struct k_mutex prv_mutex;

static inline int prv_lock(void)
{
    return k_mutex_lock(&prv_mutex, K_FOREVER);
}

static inline void prv_unlock(void)
{
    k_mutex_unlock(&prv_mutex);
}

int zephyr_in_con_store(struct bt_conn * conn)
{
    size_t i;

    if(prv_lock() != 0) return -1;

    for(i=0;i<prvTBL_SZ; i++)
    {
        if(prv_db[i].conn != NULL)
        {
            continue;
        }

        /* Reference it */
        bt_conn_ref(conn);

        /* Set the connection */
        prv_db[i].conn = conn;
        /* restart the disconnection timer */
        prv_restart_disconnect_timer();

        prv_unlock();
        return 0;
    }
    prv_unlock();
    return -1;
}

/**
 * @brief 
 * 
 * @param conn 
 * @return 0 if it contains, non zero otherwise 
 */
int zephyr_in_con_contains(struct bt_conn * conn)
{
    size_t i;

    if(prv_lock() != 0) return -1;

    for(i=0;i<prvTBL_SZ; i++)
    {
        /* Just compare */
        if(prv_db[i].conn != conn)
        {
            continue;
        }

        prv_unlock();
        return 0;
    }
    prv_unlock();
    return -1;
}

void zephyr_in_con_remove(struct bt_conn * conn)
{
    size_t i;

    if(prv_lock() != 0) return;

    for(i=0;i<prvTBL_SZ; i++)
    {
        /* Compare */
        if(prv_db[i].conn != conn)
        {
            continue;
        }

        /* Remove it */
        prv_db[i].conn = NULL;

        /* Unreference it */
        bt_conn_unref(conn);

        prv_unlock();
        return;
    }
    prv_unlock();
}

int zephyr_in_con_size(void)
{
    size_t i;
    size_t sz = 0;

    if(prv_lock() != 0) return -1;

    for(i=0;i<prvTBL_SZ; i++)
    {
        /* Check it is used */
        if(prv_db[i].conn != NULL)
        {
            continue;
        }

        sz++;
    }
    prv_unlock();
    return sz;
}