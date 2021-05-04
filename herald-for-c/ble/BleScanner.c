/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "logger/herald_logger.h"
#include "ble/BleOsScanner.h"
#include "ble/BleScanner.h"
#include "data_type/DataTypes.h"

#include <zephyr.h>


/**
 * @brief 
 * 
 * @param manufacturer_data 
 * @param data_len 
 * @param pseudo_address 
 * @return 0 for success 
 */
static int prv_get_pseudo_device_address(const Data_t * manufacturer_data,
    BleAddress_t * pseudo_address)
{
    uint16_t manufacturer_id;

    /* Check length correct for a manufacturer ID and pseudo device address */
    if(manufacturer_data->size != 2 + 6)
    {
        return -1;
    }

    /* Get the manufacturer ID */
    memcpy(&manufacturer_id, manufacturer_data->data, 2);

    /* Check the manufacuter ID is set to herald */
    if(manufacturer_id != HERALD_MANUFACTURER_ID)
    {
        return -1;
    }

    /* Get the pseudo device address */
    memcpy(&pseudo_address->val, manufacturer_data->data, 6);

    /* Return success */
    return 0;
}

void prv_scan_cb(void * module, const BleAddress_t * addr,
    Data_t * manufacturer_data, Rssi_t rssi)
{
    struct scan_results_message msg;
    int err;

    BleScanner_t * self = (BleScanner_t*) module;

    assert(self);
    assert(self->scan_res_queue);

    /* Copy the MAC address */
    BleAddress_copy(&msg.addr, addr);

    /* Get pseudo device address from manufacturer data */
    err = prv_get_pseudo_device_address(manufacturer_data, &msg.pseudo);
    
    if(err)
    {
        /* Use the MAC address */
        BleAddress_copy(&msg.pseudo, addr);
    }

    /* Add the RSSI */
    msg.rssi = rssi;

    /* Send the message */
    err = k_msgq_put(self->scan_res_queue, &msg, K_NO_WAIT);

    if(err)
    {
        LOG_ERR("Could not add scan result");
    }
}

int BleScanner_init(BleScanner_t * self, struct k_msgq * scan_res_queue)
{
    /* Set the scan queue */
    self->scan_res_queue = scan_res_queue;

    /* Initialize the scanner and filters */
    return BleOsScanner_init(prv_scan_cb, (void*) self);
}

int BleScanner_start(BleScanner_t * self)
{
    /* Start the scanner, it will call the CB with all results */
    return BleOsScanner_start();
}

int BleScanner_stop(BleScanner_t * self)
{
    return BleOsScanner_stop();
}