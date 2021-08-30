/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <assert.h>

#include <zephyr.h>
#include <settings/settings.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "ble/BleOsDevice.h"
#include "ble/zephyr/zephyr_ble.h"
#include "logger/herald_logger.h"

int BleOsDevice_init(void)
{
    int err;

	/* Initialize connections module */
	zephyr_con_manager_init();

    /* Enable bluetooth, pass NULL cb (wait here until done) */
	err = bt_enable(NULL);

    /* Error check */
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return err;
	}

    /* Register connection callbacks */
	zephyr_connection_register_callbacks();

    /* Load settings if used */
	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}

    return 0;
}