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


#define APPLE_MANUFACTURER_ID 0x004C

/**
 * \brief 
 * 
 * \param attr 
 * \param len 
 * \param data 
 * \return 1 if it requres a read, 0 if it doesn't 
 */
static int prv_apple_attr_requires_read(uint8_t attr, uint8_t len, uint8_t * data)
{
    switch(attr)
    {
        /* Device type */
        case 0x10:

            /* The length should not be zero */
            if(len == 0)
            {
                return 1;
            }

            /* If 02 follows, a read is not required */
            if(*data == 0x02)
            {
                return 0;
            }

            /* Check last byte of the attribute */
            if(data[len-1] == 0x04 || data[len-1] == 0x14)
            {
                /* Doesn't require a read */
                return 0;
            }

            /* A read is required */
            return 1;
        
        /* Old apple device */
        case 0x01:
            /* Check the length */
            if(len != 16)
            {
                /* Requires a read */
                return 1;
            }

            /* Check all zeros */
            while(len > 0)
            {
                /* Check all bytes starting from the end */
                len--;
                if(data[len] != 0x00)
                {
                    return 1;
                }
            }
            /* All zeros were found, doesn't require a read */
            return 0;

        case 0x05:
        case 0x07:
        case 0x09:
        case 0x00:
        case 0x06:
        case 0x08:
        case 0x03:
        case 0x0C:
        case 0x0D:
        case 0x0F:
        case 0x0E:
        case 0x0B:
            return 0;

        /* Anything else will require a read */
        default:
            return 1;
    }

    /* Cannot get here */
}

/**
 * @brief Check if an apple device needs to be conencted to
 * 
 * @param manufacturer_data The manufacturer data not including manufacture ID 
 * @return return 0 if we do NOT need to connect 
 */
static int prv_check_apple_manufacturer_data(uint8_t * data, size_t size)
{
    uint8_t attr;
    uint8_t attr_len;

    while(size > 2)
    {
        /* Get attribute */
        attr = *data;
        data++;
        size--;

        /* Get the length */
        attr_len = *data;
        data++;
        size--;

        /* Check the data size accomidates */
        if(attr_len > size)
        {
            LOG_ERR("Apple attribute length to long!");
            return -1;
        }

        if(prv_apple_attr_requires_read(attr, attr_len, data) != 0)
        {
            /* We require to connect */
            return 1;
        }

        /* Increment the data */
        data += attr_len;
        size -= attr_len;
    }

    /* Check extra data */
    if(size)
    {
        LOG_WRN("Extra data in apple manufacturer data!");
        /* Something went wrong, require a read */
        return 1;
    }

    /* If we are here, we do not require to connect to this device */
    return 0;
}

void prv_scan_cb(void * module, const BleAddress_t * addr,
    const Data_t * manufacturer_data, Rssi_t rssi, uint8_t status)
{
    struct scan_results_message msg;
    int err;
    uint8_t * data = manufacturer_data->data;
    size_t sz = manufacturer_data->size;

    BleScanner_t * self = (BleScanner_t*) module;

    assert(self);
    assert(self->scan_res_queue);

    /* Copy the MAC address */
    BleAddress_copy(&msg.addr, addr);

    /* Logic to find if the device could be herald */
    msg.could_be_herald = 
        /* First check it is connectable */
        (status & BleOsScanner_STATUS_CONNECTABLE)
        &&
        (
            /* Check Herald UUID was found */
            (
                status & BleOsScanner_STATUS_HERALD_UUID_FOUND
            )
            ||
            /* Check apple data */
            (
                /* Check apple manufacturer ID */
                sz >= 2 && *((uint16_t*)data) == APPLE_MANUFACTURER_ID
                &&
                /* Check the data for markers in apple manufacturer data */
                prv_check_apple_manufacturer_data(data+2, sz-2) != 0
            )
        );

    /* Check for Pseudo ID, first check Herald manufacturer data */
    if(sz >= 2 && *((uint16_t*)data) == HERALD_MANUFACTURER_ID)
    {
        /* Check size */
        if(sz >= 2+6)
        {
            /* Check exact size */
            if(sz != 2 + 6)
            {
                LOG_WRN("Herald manufacturer ID to long! Using anyway!");
            }

            /* Get the pseudo device address from manufacturer data */
            memcpy(&msg.pseudo.val, data + 2, 6);
        }
        else
        {
            LOG_WRN("Herald manufacturer ID too short! Not using!");
            /* Use the MAC address */
            BleAddress_copy(&msg.pseudo, addr);
        }
    }
    else
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