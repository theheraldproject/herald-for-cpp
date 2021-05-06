/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __ZEPHYR_BLE_H__
#define __ZEPHYR_BLE_H__

#include <bluetooth/conn.h>
#include "data_type/DataTypes.h"
#include "logger/herald_logger.h"
#include "ble/BleErrCodes.h"

/* Herald advertising parameters */
#define HRLD_ADV_INT_MIN ( \
    (\
        CONFIG_HERALD_ADVERTISING_INTERVAL - \
        CONFIG_HERALD_ADVERTISING_INTERVAL_DEVIATION \
    ) / 0.625)

#define HRLD_ADV_INT_MAX ( \
    (\
        CONFIG_HERALD_ADVERTISING_INTERVAL + \
        CONFIG_HERALD_ADVERTISING_INTERVAL_DEVIATION \
    ) / 0.625)

/* Herald scan parameters */
#define HRLD_SCAN_INTERVAL (CONFIG_HERALD_SCAN_INTERVAL_MS / 0.625)
#define HRLD_SCAN_WINDOW (CONFIG_HERALD_SCAN_WINDOW_MS / 0.625)

/* Herald IDs */
extern struct bt_uuid_128 herald_uuid;
extern struct bt_uuid_128 herald_payload_uuid;
extern struct bt_uuid_128 herald_write_uuid;

static inline void zephyr_addr_from_hrld_addr(bt_addr_le_t * nrf_addr, const BleAddress_t * addr)
{
    memcpy(nrf_addr->a.val, addr->val, 6);
    nrf_addr->type = BT_ADDR_LE_RANDOM;
}

static inline void hrld_addr_from_zephyr_addr(BleAddress_t * addr,
    const bt_addr_le_t * nrf_addr)
{
    /* Copy the MAC address */
    memcpy(addr->val, nrf_addr->a.val, 6);
}

static inline void hrld_addr_from_conn(BleAddress_t * addr, const struct bt_conn * conn)
{
    /* Get the address of the connected device */
    hrld_addr_from_zephyr_addr(addr, bt_conn_get_dst(conn));
}

/* Scanner */
void BleZephyrScanner_cb(const bt_addr_le_t *addr, int8_t rssi, Data_t * manufacturer_data,
    uint8_t status);
void BleZephyrScan_allow(void);
void BleZephyrScan_disallow(void);
int zephyr_scan_init(void);
int zephyr_scan_start(void);
int zephyr_scan_stop(void);


/* Advertiser */
void BleZephyrAdvertise_disallow(void);
void BleZephyrAdvertise_allow(void);
int zephyr_advertise_start(void);
int zephyr_advertise_stop(void);


/* Reader */
int BleZephyrReader_callback(struct bt_conn * conn, uint8_t * data,
    size_t len, int status);
void BleZephyrReader_err_callback(struct bt_conn * conn, int status);
void BleZephyrReader_disconnection_cb(struct bt_conn * conn);

/* Transmitter */
int BleZephyrTransmitter_should_accept_connection_cb(struct bt_conn * conn);
int BleZephyrTransmitter_get_payload_cb(struct bt_conn * conn, Data_t * data);

/* Connections */
int zephyr_connection_create(const bt_addr_le_t * addr, struct bt_conn ** conn);
void zephyr_connection_register_callbacks(void);
int zephyr_connection_disconnect(struct bt_conn * con);

/* GATT */
void zephyr_gatt_start_discovery(struct bt_conn * conn);

/* Connection manager */
void zephyr_con_manager_init(void);
int zephyr_con_manager_add(struct bt_conn * conn);
int zephyr_con_manager_contains(struct bt_conn * conn);
void zephyr_con_manager_remove(struct bt_conn * conn);
struct bt_gatt_read_params *
    zephyr_con_manager_get_read_params(struct bt_conn * conn);
int zephyr_con_manager_size(void);

/* Incoming connection manager */
int zephyr_in_con_store(struct bt_conn * conn);
int zephyr_in_con_contains(struct bt_conn * conn);
void zephyr_in_con_remove(struct bt_conn * conn);

#endif /* __ZEPHYR_BLE_H__ */