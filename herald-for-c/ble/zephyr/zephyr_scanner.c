/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/scan.h>

#include "ble/zephyr/zephyr_ble.h"

/**
 * \brief Get manufacturer data callback, this is passed to bt_data_parse
 * 
 * \param data The bt_data
 * \param user_data pointer to a structer of type BleAddress,
 *                  to be populated with manufacturer data
 * \return false if the callback should be called again with next data
 */
static bool prv_get_manufacturer_data_cb(struct bt_data * data, Data_t * manufacturer_data)
{
    /*
     * struct bt_data
     * {
     *  uint8_t type \\ see 
     *  uint8_t data_len
     *  const uint8_t * data
     * }
     * 
     * For possible valuse of type see include/bluetooth/gap.h:29 or Bluetooth SIG docs
     */

    /* make sure it is bluetooth data */
    if(data->type != BT_DATA_MANUFACTURER_DATA)
    {
        /* The manufacturer data sectin has not been found yet,
        keep calling this callback with new data */
        return true;
    }

    manufacturer_data->size = data->data_len;
    /* WARN: Casting from const to non const */
    manufacturer_data->data = (uint8_t*)data->data;

    /* The manufacturer data has been found, stop calling this callback with new data */
    return false;
}

/**
 * \brief Scan callback for any discovered LE device
 * 
 * \param addr The advertiser LE address and type
 * \param rssi Strength of the advertiser signal
 * \param adv_type Type of advertising response from advertiser
 * \param buf buffer containing the advertising data
 */
static void prv_scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
  struct net_buf_simple *buf)
{
    Data_t manufacturer_data;

    /* Zero the manufacuter_data */
    memset(&manufacturer_data, 0, sizeof(Data_t));

    /* Get the manufacturer data
    Use the zephyr provided method and call the provided callback on all advertised data
    The callback provided will populate the provided Data_t with the manufacturer data if found */
    bt_data_parse(buf,
        (bool(*)(struct bt_data*, void*) ) prv_get_manufacturer_data_cb,
        &manufacturer_data);

    BleZephyrScanner_cb(addr, rssi, &manufacturer_data);
}

/**
 * \brief Called on a filter match
 * 
 * Auto connection is enabled so we do not need to do anything here
 * Just log for now
 * 
 * \param device_info 
 * \param filter_match The filter status
 * \param connectable 
 */
static void scan_filter_match(struct bt_scan_device_info * device_info,
    struct bt_scan_filter_match * filter_match, bool connectable)
{
	// char addr[BT_ADDR_LE_STR_LEN];

	// bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	// LOG_INF("Address: %s connectable: %d, type: %d", log_strdup(addr),
    //     connectable, device_info->recv_info->addr->type);

    /* Check it is connectable */
    if(connectable == false)
    {
        return;
    }

	/* Call the traditional scan callback */
	prv_scan_cb(device_info->recv_info->addr, device_info->recv_info->rssi,
		device_info->recv_info->adv_type, device_info->adv_data);
}

/**
 * \brief Connection error
 * 
 * Called when there is an error connecting
 * 
 * \param device_info 
 */
static void scan_connecting_error(struct bt_scan_device_info * device_info)
{
	LOG_WRN("scan_connecting_error(): Could not initialize connection");
}

/**
 * \brief Called when we are starting to connect to a device
 * 
 * \param device_info 
 * \param conn 
 */
static void scan_connecting(struct bt_scan_device_info *device_info,
	struct bt_conn *conn)
{
    LOG_INF("scan_conecting(): Connecting...");
}

/* This macro initializes the scan callback */
BT_SCAN_CB_INIT(
    /* Scan callback variable name */
    nrf_scan_cb,
    /* The match function, called everytime a match is found */
    scan_filter_match,
    /* The no match function, called if the item does not match */
    NULL,
    /* Called when there is an error starting a connection */
    scan_connecting_error,
    /* Called when we are starting to connect to a device */
    scan_connecting
);

static const struct bt_le_scan_param scan_params = 
{
	/* Passive scan */
	.type       = BT_LE_SCAN_TYPE_PASSIVE,
	/* Scan for everything but filter duplicates */
	.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
	/* Set scan interval, 0x60: 60ms */
	.interval   = HRLD_SCAN_INTERVAL,
	/* Set scan window, 0x30: 30ms */
	.window     = HRLD_SCAN_WINDOW
};

/* The scan init structure */
static const struct bt_scan_init_param prv_scan_init = 
{
	/* Use scan params */
	.scan_param = &scan_params,
    /* Start a connection on a filter match */
    .connect_if_match = 0,
	/* Use default connection parameters */
	.conn_param = NULL
};

static uint8_t apple_manufacturer_id[2] = {0x4C,0x00};

/**
 * \brief Initialize the scanner and initialize the filters 
 * 
 * \return error code, 0 for no error 
 */
int zephyr_scan_init(void)
{
	int err;
	
    /* Initialize the scanner */
	bt_scan_init(&prv_scan_init);

    /* Register the scan callback structure */
	bt_scan_cb_register(&nrf_scan_cb);

    /* Add a filter for the Herald service UUID */
	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, &herald_uuid);

    /* Error check */
	if (err)
    {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}

    /* Add a filter for Apple devices */

    struct bt_scan_manufacturer_data apple_filter;
    apple_filter.data = apple_manufacturer_id;
    apple_filter.data_len = 2;

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_MANUFACTURER_DATA, &apple_filter);

    /* Error check */
	if (err)
    {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}



    /* Enable the scan filter for UUID and Manufacturer data
    Only require one filter match (match_all=false) */
	err = bt_scan_filter_enable(
        BT_SCAN_UUID_FILTER | BT_SCAN_MANUFACTURER_DATA_FILTER,
        false);

    /* Error check */
	if (err)
    {
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}

	LOG_INF("Scan module initialized");
	return err;
}

/**
 * Start scanning in passive mode
 */
int zephyr_scan_start(void)
{
	int err;

	/* Start scan in passive mode */
	err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);

    /* Error check */    
	if (err)
    {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return err;
	}

	//LOG_INF("Scanning successfully started");
    return err;
}

int zephyr_scan_stop(void)
{
    return bt_le_scan_stop();
}