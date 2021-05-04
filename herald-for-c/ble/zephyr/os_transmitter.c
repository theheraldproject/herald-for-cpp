/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/BleOsTransmitter.h"
#include "ble/zephyr/zephyr_ble.h"

void * prv_module = NULL;
static BleOsTransmitter_allow_connection_cb prv_allow_connection_cb = NULL;
static BleOsTransmitter_get_payload_cb prv_get_payload_cb = NULL;
static BleOsTransmitter_received_payload_cb prv_received_payload_cb = NULL;

int BleOsTransmitter_init(void * module,
    BleOsTransmitter_allow_connection_cb allow_connection_cb,
    BleOsTransmitter_get_payload_cb get_payload_cb,
    BleOsTransmitter_received_payload_cb received_payload_cb)
{
    prv_module = module;
    prv_allow_connection_cb = allow_connection_cb;
    prv_get_payload_cb = get_payload_cb;
    prv_received_payload_cb = received_payload_cb;

    return 0;
}

int BleZephyrTransmitter_should_accept_connection_cb(struct bt_conn * conn)
{
    BleAddress_t addr;

    assert(prv_allow_connection_cb);

    /* Get the address */
    hrld_addr_from_conn(&addr, conn);

    /* Call the callback */
    return prv_allow_connection_cb(prv_module, &addr);
}

int BleZephyrTransmitter_get_payload_cb(struct bt_conn * conn, Data_t * data)
{
    BleAddress_t addr;
    assert(prv_get_payload_cb);
    /* Get the address */
    hrld_addr_from_conn(&addr, conn);

    return prv_get_payload_cb(prv_module, &addr, data);
}