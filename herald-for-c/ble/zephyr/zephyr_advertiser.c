/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gatt.h>
#include "ble/zephyr/zephyr_ble.h"


/** 
 * Peripheral requests a read from herald payload
 * 
 * \param conn The connection object
 * \param attr The attribute being read
 * \param buf The buffer to place the data
 * \param len The length of the data
 * \param offset The offset to start reading from
 * 
 * \return The number of bytes populated in the buffer, negative in error
 */
static ssize_t herald_payload_read_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
    Data_t data;
    int err;

    /* Get the payload */
    err = BleZephyrTransmitter_get_payload_cb(conn, &data);

    /* Error check */
    if(err)
    {
        LOG_ERR("Could not serve payload");
        return 0;
    }

	/* Read the attribute data into the buffer */
	return bt_gatt_attr_read(conn, attr, buf, len, offset, data.data,
				 data.size);
}

/**
 * Peripheral attempts to write to write characteristic
 * 
 * \param conn The connection object
 * \param attr The attribute that is being written too
 * \param buf The buffer with the data
 * \param len The number of bytes in the buffer
 * \param offset The number offset to start writing from 
 * \param flags Flags (BT_GATT_WRITE_*)
 * 
 * \return Number of bytes written
 * \return BT_GATT_ERR() in case of error
 */
static ssize_t herald_write_characteristic_write_cb(struct bt_conn *conn, 
	const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset,
	uint8_t flags)
{
	// uint8_t *value = attr->user_data;

	int i;

	/* For now, log the data */
	printk("Write: ");
	for(i=0 ; i<len; i++)
	{
		printk("%02X ", ((uint8_t*) buf)[i]);
	}
	printk("\r\n");

	/* We wrote all data, return the entire length */
	return len;
}

/* Advertising data */
static const struct bt_data ad[] = {
	/* Data types */
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	
	/* Manufacturer Data */
	BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA, 0xFF, 0xFA),
	
	/* 128 bit service UUIDS */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		/* Herald */
		0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 
		0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42),
};


/* Herald service decleration
This macro statically defines and registers the herald service */
BT_GATT_SERVICE_DEFINE(herald_service,

	/* The herald service */
	BT_GATT_PRIMARY_SERVICE(&herald_uuid),

	/* The Payload characteristic */
	BT_GATT_CHARACTERISTIC(
		/* Herald UUID */
		&herald_payload_uuid.uuid,
		/* Allow reading only */
		BT_GATT_CHRC_READ,
		/* Access permissions */
		BT_GATT_PERM_READ,
		/* The read callback */
		herald_payload_read_cb,
		/* The write callback */
		NULL,
		/* The initial value of the characteristic */
		NULL
	),

	/* The write/indicate characteristic */
	BT_GATT_CHARACTERISTIC(
		/* Write characteristic UUID */
		&herald_write_uuid.uuid,
		/* Allow writing with acknowledgement */
		BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
		/* No encryption needed */
		BT_GATT_PERM_WRITE,
		/* Read callback */
		NULL,
		/* Write callback */
		herald_write_characteristic_write_cb,
		/* Initial value */
		NULL
	)
);



struct bt_le_adv_param adv_param = 
	BT_LE_ADV_PARAM_INIT(
		/* Advertise as connectable, AND use the device GAP name */
		BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
        // BT_LE_ADV_OPT_USE_NAME,
		/* Set advertising interval min, 0xa0: 100ms */
		HRLD_ADV_INT_MIN,
		/* Set advertising interval max, 0xf0: 150ms */
		HRLD_ADV_INT_MAX,
		/* Peer address set to NULL,
		NULL: undirected advertising or address of peer for directed advertising */
		NULL
	);



int zephyr_advertise_start(void)
{
    /* Start advertising
	First parameter is the struct bt_le_adv_param
	*/
	return bt_le_adv_start(
		/* Advertising options: connectable, use GAP name,
		interval min:100ms, intervale max: 150ms */
		&adv_param,
		/* Set advertising data */
		ad, ARRAY_SIZE(ad),
		/* No scan response data */
		NULL, 0);
}

int zephyr_advertise_stop(void)
{
    return bt_le_adv_stop();
}
