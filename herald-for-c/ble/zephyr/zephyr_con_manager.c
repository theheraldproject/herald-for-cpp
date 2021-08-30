/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>

#include <bluetooth/gatt.h>
#include "ble/zephyr/zephyr_ble.h"

#include "logger/herald_logger.h"

struct herald_initiated_con
{
    uint8_t used;
    /**< 1 for used, 0 for not used */
	struct bt_conn * conn;
    /**< The connection handle */
    struct bt_gatt_read_params read_params;
    /**< The read params */
};

static struct herald_initiated_con
    herald_conns[CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME];

static struct k_mutex prv_mutex;

static inline int prv_lock(void)
{
    return k_mutex_lock(&prv_mutex, K_FOREVER);
}

static inline void prv_unlock(void)
{
    k_mutex_unlock(&prv_mutex);
}

void zephyr_con_manager_init(void)
{
    /* Clear the structure */
    memset(herald_conns, 0,
        sizeof(struct herald_initiated_con) * CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME);
    /* Initialize the mutex */
    k_mutex_init(&prv_mutex);
}

/**
 * \brief Store a connection initiated by zephyr 
 * 
 * The connection should already be connected
 * 
 * \param conn A connection that has been referenced
 * \return 0 for success 
 */
int zephyr_con_manager_add(struct bt_conn * conn)
{
    size_t i;

    /* Lock */
    if(prv_lock() != 0) return -1;


    for(i=0;i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME; i++)
    {
        if(herald_conns[i].used != 0)
        {
            continue;
        }

        /* Add it */
        herald_conns[i].used = 1;
        herald_conns[i].conn = conn;

        prv_unlock();
        return 0;
    }

    /* No free spots */
    prv_unlock();
    return -1;
}

struct bt_gatt_read_params *
    zephyr_con_manager_get_read_params(struct bt_conn * conn)
{
    size_t i;

    if(prv_lock() != 0) return NULL;

    for(i=0;i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME; i++)
    {
        if(herald_conns[i].used == 0)
        {
            continue;
        }

        /* Compare it */
        if(conn != herald_conns[i].conn)
        {
            continue;
        }

        /* It does contain it */
        prv_unlock();
        return &herald_conns[i].read_params;
    }

    /* Connection not found */
    prv_unlock();
    return NULL;
}

/**
 * \brief Find a saved connection 
 * 
 * \param conn 
 * \return 0 if it contains the connection
 */
int zephyr_con_manager_contains(struct bt_conn * conn)
{
    if(zephyr_con_manager_get_read_params(conn) == NULL)
    {
        /* Does not contain it */
        return -1;
    }
    /* It does exist */
    return 0;
}

/**
 * \brief Remove a connection reference 
 * 
 * \param conn 
 */
void zephyr_con_manager_remove(struct bt_conn * conn)
{
    size_t i;

    if(prv_lock() != 0) return;

    for(i=0;i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME;i++)
    {
        if(herald_conns[i].used == 0)
        {
            continue;
        }

        if(conn != herald_conns[i].conn)
        {
            continue;
        }

        /* Unreference it */
        bt_conn_unref(conn);

        /* Remove it */
        herald_conns[i].used = 0;
        
        /* Return, we removed it */
        prv_unlock();
        return;
    }

    prv_unlock();
    LOG_ERR("Attempt to remove unknown connection ");
}


int zephyr_con_manager_size(void)
{
    size_t i;
    size_t sz = 0;

    if(prv_lock() != 0) return -1;

    for(i=0;i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME;i++)
    {
        if(herald_conns[i].used == 0)
        {
            continue;
        }

        sz++;
    }
    prv_unlock();
    return sz;
}
