/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "logger/herald_logger.h"
#include "ble/zephyr/zephyr_ble.h"

/**
 * \brief Connection event
 * 
 * The device got thought the filter and started a connection 
 * 
 * @param conn 
 * @param conn_err 
 */
static void connected(struct bt_conn * conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];

    /* Get the address for logging */
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	
	/* Check if it is a connection we initiated */
	if(zephyr_con_manager_contains(conn) == 0)
	{
		/* Log */
		LOG_INF("Outgoing connection : %s", log_strdup(addr));

		if(conn_err)
		{
			LOG_INF("Outgoing connection error!");
			/* Run the callback */
			BleZephyrReader_err_callback(conn, BleErr_ERR_CONNECTING);
			return;
		}

		/* We initiated it, start the GATT discovery */
		zephyr_gatt_start_discovery(conn);
	}
	else
	{
		/* Log */
		LOG_INF("Incoming connection : %s", log_strdup(addr));

		if(conn_err)
		{
			LOG_INF("Incoming connection error");
			return;
		}

		BleZephyrScan_disallow();
        BleZephyrAdvertise_disallow();

		/* Store it */
		if(zephyr_in_con_store(conn) != 0)
		{
			LOG_WRN("Could not store incoming connection! Disconnecting");
			/* We should not accept it, disconnect */
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			return;
		}

		/* Check if we should allow the connection */
		if(BleZephyrTransmitter_should_accept_connection_cb(conn) != 0)
		{
			LOG_DBG("Denying connection!");
			/* We should not accept it, disconnect */
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			return;
		}
		LOG_DBG("Allowing connection");
	}
}

/**
 * @brief Disconnection event
 * 
 * @param conn 
 * @param reason 
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

    /* Get the address for logging */
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    /* Log */
	LOG_INF("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	/* Check if it was an outgoing connection */
	if(zephyr_con_manager_contains(conn) == 0)
	{
		BleZephyrReader_disconnection_cb(conn);
	}
	/* Check if it was an incoming connection */
	else if(zephyr_in_con_contains(conn) == 0)
	{
		/* Remove it */
		BleZephyrScan_allow();
        BleZephyrAdvertise_allow();
		/* Signal a write done,
		has no effect if the device did not write */
		BleZephyrTransmitter_received_data_cb(conn, NULL, 0, -1);
		zephyr_in_con_remove(conn);
	}
	else
	{
		BleZephyrScan_allow();
        BleZephyrAdvertise_allow();
		LOG_ERR("Stray connection!");
	}
}

static struct bt_conn_cb conn_callbacks =
{
    .connected = connected,
    .disconnected = disconnected
};

/**
 * Register the connection callbacks
 */
void zephyr_connection_register_callbacks(void)
{
    bt_conn_cb_register(&conn_callbacks);
}

struct bt_conn_le_create_param create_params = 
	BT_CONN_LE_CREATE_PARAM_INIT(
		BT_CONN_LE_OPT_NONE,
		BT_GAP_SCAN_FAST_INTERVAL,
		BT_GAP_SCAN_FAST_INTERVAL
  	);

struct bt_le_conn_param conn_params =
	BT_LE_CONN_PARAM_INIT(
		BT_GAP_INIT_CONN_INT_MIN, BT_GAP_INIT_CONN_INT_MAX, 0, 400
		// //12, 12 // aka 15ms, default from apple documentation
		// 0x50, 0x50, // aka 80ms, from nRF SDK LLPM sample
		// 0, 400
	);

int zephyr_connection_create(const bt_addr_le_t * addr, struct bt_conn ** conn)
{
	char addr_str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

	LOG_INF("Starting connection: Address: %s, type: %d", log_strdup(addr_str),
         addr->type);

	/* Create the connection */
	return bt_conn_le_create(addr, &create_params, &conn_params, conn);
}

int zephyr_connection_disconnect(struct bt_conn * conn)
{
	return bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}