/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/zephyr/zephyr_ble.h"

#define CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE 3

struct in_con_s
{
    struct bt_conn * conn;
};

static struct in_con_s prv_db[CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE];

void zephyr_in_cons_init(void)
{
    /* Zero the db */
    memset(prv_db, 0, sizeof(struct in_con_s) * CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE);
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
    int i;

    if(prv_lock() != 0) return -1;

    for(i=0;i<CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE; i++)
    {
        if(prv_db[i].conn != NULL)
        {
            continue;
        }

        /* Reference it */
        bt_conn_ref(conn);

        /* Set the connection */
        prv_db[i].conn = conn;
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
    int i;

    if(prv_lock() != 0) return -1;

    for(i=0;i<CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE; i++)
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
    int i;

    if(prv_lock() != 0) return;

    for(i=0;i<CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE; i++)
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
    int i;
    size_t sz = 0;

    if(prv_lock() != 0) return -1;

    for(i=0;i<CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE; i++)
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