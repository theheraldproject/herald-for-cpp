/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>

#include <bluetooth/bluetooth.h>

#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>

#include "ble/zephyr/zephyr_ble.h"

/**
 * \brief Callback to read a characteristics value
 * 
 * \param conn The connection object
 * \param err ATT error code
 * \param params Read parameters used
 * \param data The attrubute value data, NULL means read has completed
 * \param length The attribute value length
 * \return BT_GATT_ITER_CONTINUE if should continue to the next attribute
 * \return BT_GATT_ITER_STOP to stop
 */
static uint8_t gatt_read_cb(struct bt_conn * conn, uint8_t err, struct bt_gatt_read_params * params,
	const void * data, uint16_t length)
{

	// TODO: Should we check err? Haven't found documentation on this
	
	if(data == NULL)
	{
        /* Run the callback */
		BleZephyrReader_err_callback(conn, BleErr_OK);

		return BT_GATT_ITER_STOP;
	}

    /* Run the callback, and check there is an error */
	if(BleZephyrReader_callback(conn, (uint8_t*) data, length, BleErr_OK) != 0)
	{
		return BT_GATT_ITER_STOP;
	}
	
	return BT_GATT_ITER_CONTINUE;
}

static void discovery_complete(struct bt_gatt_dm * dm, void *context)
{
	int err;

    struct bt_conn * conn = bt_gatt_dm_conn_get(dm);

    /* Attempt to read the herald payload characteristic */
	const struct bt_gatt_dm_attr * attr = 
		bt_gatt_dm_char_by_uuid(dm, (const struct bt_uuid *) &herald_payload_uuid);

	struct bt_gatt_chrc * ch = bt_gatt_dm_attr_chrc_val(attr);

	if(ch == NULL)
	{
		LOG_WRN("Could not find payload characteristic in herald service");
		bt_gatt_dm_data_release(dm);
        /* Run the callback with error */
		BleZephyrReader_err_callback(conn, BleErr_ERR_HERALD_PAYLOAD_NOT_FOUND);
		return;
	}

	/* Get the read parameters for the connection */
	struct bt_gatt_read_params * read_params =
		zephyr_con_manager_get_read_params(conn);
	
	/* Error check */
	if(read_params == NULL)
	{
		LOG_ERR("Could not get read params!");
		bt_gatt_dm_data_release(dm);
		BleZephyrReader_err_callback(conn, BleErr_SYSTEM);
		return;
	}

	/* Setup the read parameters */
	read_params->func = gatt_read_cb;
	read_params->handle_count = 1;
	read_params->single.handle = ch->value_handle;
	read_params->single.offset = 0x00;

	err = bt_gatt_read(bt_gatt_dm_conn_get(dm), read_params);

    if(err)
    {
        LOG_ERR("Error starting GATT read!");
		bt_gatt_dm_data_release(dm);
        /* Run the callback */
        BleZephyrReader_err_callback(conn, BleErr_SYSTEM);
		return;
    }

	bt_gatt_dm_data_release(dm);
}

static void discovery_service_not_found(struct bt_conn *conn, void *context)
{
    /* Run the callback */
    BleZephyrReader_err_callback(conn, BleErr_ERR_HERALD_SERVICE_NOT_FOUND);
}

static void discovery_error(struct bt_conn *conn, int err, void *context)
{
	LOG_WRN("Error while discovering GATT database: (%d)", err);

    /* Run the callback */
    BleZephyrReader_err_callback(conn, BleErr_ERR_GATT_DISCOVERY);
}

struct bt_gatt_dm_cb discovery_cb =
{
	.completed         = discovery_complete,
	.service_not_found = discovery_service_not_found,
	.error_found       = discovery_error,
};

/* Start discovery of GATT services */
void zephyr_gatt_start_discovery(struct bt_conn * conn)
{
	int err;

    /* Start discovery of the herald service */
	err = bt_gatt_dm_start(conn,
        (const struct bt_uuid *) &herald_uuid,
        &discovery_cb,
        NULL
    );
    
	if (err)
    {
		LOG_ERR("could not start the discovery procedure, error "
			"code: %d", err);
		/* Run the callback */
        BleZephyrReader_err_callback(conn, BleErr_SYSTEM);
	}
}