/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/zephyr/zephyr_ble.h"
#include "ble/BleOsReader.h"

static BleOsReader_payload_cb_t prv_payload_cb;
static BleOsReader_connection_done_cb_t prv_done_cb;
static void * prv_module;

void BleZephyrReader_disconnection_cb(struct bt_conn * conn)
{
    BleAddress_t addr;
    hrld_addr_from_conn(&addr, conn);
    /* Remove it from known connections */
    zephyr_con_manager_remove(conn);
    /* Restart the scanner */
    BleZephyrScan_allow();
    /* Resert advertiser */
    BleZephyrAdvertise_allow();
    /* Call the callback ending the payload read */
    prv_done_cb(prv_module, &addr);
}

int BleZephyrReader_callback(struct bt_conn * conn, uint8_t * data,
    size_t len, int status)
{
    BleAddress_t addr;
    Data_t data_st;
    assert(prv_payload_cb);

    hrld_addr_from_conn(&addr, conn);

    data_st.size = len;
    data_st.data = data;

    /* Add data */
    status = prv_payload_cb(prv_module, &addr, status, &data_st);

    if(status == BleErr_ERR_CONNECTING)
    {
        /* Call the callback directly */
        BleZephyrReader_disconnection_cb(conn);
    }
    else if(status)
    {
        /* Time to disconnect, callback will be called when disconnection happens */
        bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }

    return status;
}


void BleZephyrReader_err_callback(struct bt_conn * conn, int status)
{
    BleZephyrReader_callback(conn, NULL, 0, status);
}

int BleOsReader_init(void * module, BleOsReader_payload_cb_t payload_cb, 
    BleOsReader_connection_done_cb_t done_cb)
{
    prv_payload_cb = payload_cb;
    prv_module = module;
    prv_done_cb = done_cb;
    return 0;
}


int BleOsReader_get_payload(const BleAddress_t * addr)
{
    bt_addr_le_t zephyr_addr;
    struct bt_conn * conn;
    int err;

    /* Convert the address to Zephyr address */
    zephyr_addr_from_hrld_addr(&zephyr_addr, addr);

    /* Request a scanner stop */
    BleZephyrScan_disallow();
    /* Advertiser stop */
    BleZephyrAdvertise_disallow();

    /* Give it time to stop */
    k_sleep(K_MSEC(100));

    /* Start the process of connecting,
    callbacks in the RX thread will be used from here */
    err = zephyr_connection_create(&zephyr_addr, &conn);

    if(err)
    {
        LOG_ERR("Could not start connection! restarting scanner...");
        /* Restart the scanner */
        BleZephyrScan_allow();
        BleZephyrAdvertise_allow();
        return err;
    }

    err = zephyr_con_manager_add(conn);

    /* Error check */
	if(err)
	{
        /* Restart the scanner */
        BleZephyrScan_allow();
        BleZephyrAdvertise_allow();

		LOG_ERR("Could not add connection when creating!");
		return err;
	}

    return 0;
}