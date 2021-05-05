/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "data_type/DataTypes.h"
#include "ble/BleDevice.h"
#include "database/DatabaseDelegate.h"
#include "database/BleDatabase.h"

#include "sensor/SensorDelegate.h"
#include "sensor/BleSensor.h"
#include "ble/BleOsDevice.h"
#include "ble/BleOsDevice.h"
#include "ble/BleAdvertiser.h"
#include "ble/BleScanner.h"
#include "ble/BleErrCodes.h"

#include <zephyr.h>


#define SCAN_MSG_QUEUE_SIZE 100
#define PAYLOAD_MSG_QUEUE_SIZE 2



int BleSensor_init(BleSensor_t * self)
{
    int err;

    /* Initialize BLE on the device */
    err = BleOsDevice_init();

    if(err)
    {
        LOG_ERR("Device init!");
        return -1;
    }

    /* Initialize the reader */
    err = BleReader_init(&self->reader, self->payload_queue);

    if(err)
    {
        LOG_ERR("Reader init!");
        return -1;
    }

    /* Initialize the transmitter */
    err = BleTransmitter_init(&self->transmitter);

    if(err)
    {
        LOG_ERR("Transmitter init!");
        return -1;
    }

    /* Initialize the scanner */
    err = BleScanner_init(&self->scanner, self->scan_queue);

    if(err)
    {
        LOG_ERR("Scanner init!");
        return -1;
    }

    /* Initialize the advertiser */
    err = BleAdvertiser_init(&self->advertiser);

    if(err)
    {
        LOG_ERR("Advertiser init!");
        return -1;
    }

    return 0;
}

void BleSensor_process_payload(BleSensor_t * self)
{
    BleDevice_t * dev;
    Data_t data;
    struct payload_msg msg;

    assert(self);

    /* Wait for payloads */
    k_msgq_get(self->payload_queue, &msg, K_FOREVER);

    /* Get the device from the DB */
    dev = BleDatabase_find_create_device(self->database, &msg.pseudo);

    /* Error check */
    if(dev == NULL)
    {
        LOG_ERR("Find create. Process payloads");
        BleScanner_start(&self->scanner);
        return;
    }

    /* Check status */
    if(msg.status)
    {
        switch(msg.status)
        {
        case BleErr_ERR_CONNECTING:
            /* Record connection error */
            BleDatabase_connection_error(self->database, dev);
            LOG_DBG("Could not connect!");
            break;

        case BleErr_ERR_READING_GATT:
            /* Record connection error */
            BleDatabase_connection_error(self->database, dev);
            LOG_WRN("Error reading GATT!");
            break;

        case BleErr_ERR_SERVICE_NOT_FOUND:
            /* Record Herald error */
            BleDatabase_herald_not_found(self->database, dev);
            LOG_DBG("Service not found!");
            break;

        case BleErr_ERR_PAYLOAD_NOT_FOUND:
            /* Record Herald error */
            BleDatabase_herald_not_found(self->database, dev);
            LOG_WRN("Payload characteristic not found!");
            break;

        case BleErr_ERR_DISCOVERING_GATT:
            /* Record connection error */
            BleDatabase_connection_error(self->database, dev);
            LOG_WRN("Error discovering GATT!");
            break;

        case BleErr_ERR_PAYLOAD_TO_BIG:
            /* Record Herald error */
            BleDatabase_herald_not_found(self->database, dev);
            LOG_WRN("Payload too large for allocated space!");
            break;

        default:
            /* Record a connection error */
            BleDatabase_connection_error(self->database, dev);
            LOG_ERR("Unknown error! (%d)", msg.status);
        }

        /* Update the device state */
        BleDatabase_set_state(self->database, dev, BleDevice_stateIDLE);
        return;
    }

    /* Create the data structure */
    data.size = msg.payload_sz;
    data.data = msg.payload;

    /* Record the payload */
    BleDatabase_read_payload(self->database, &msg.pseudo, dev, &data);

    /* Update device state to idle */
    BleDatabase_set_state(self->database, dev, BleDevice_stateIDLE);
}

void BleSensor_process_scan(BleSensor_t * self)
{
    BleDevice_t * dev;
    struct scan_results_message scan_msg;
    int err;

    assert(self);
    
    /* Wait for scan result */
    err = k_msgq_get(self->scan_queue, &scan_msg, K_FOREVER);

    /* Check if scan result was found */
    if(err)
    {
        return;
    }

    /* Run the didMeasure callback */
    BleDatabase_record_rssi(self->database, &scan_msg.pseudo, NULL, scan_msg.rssi);

    /* Check if it is forsure not a herald device, no need to include in DB if it is */
    if(scan_msg.could_be_herald == 0)
    {
        return;
    }

    /* Add to DB */
    dev = BleDatabase_find_create_device(self->database, &scan_msg.pseudo);

    if(dev == NULL)
    {
        LOG_ERR("Find create. Process scans");
        return;
    }

    /* Record the scan */
    BleDatabase_scanned(self->database, dev);

    /* Check the device has been marked as `do not connect` */
    if(BleDatabase_should_connect(self->database, dev) == 0)
    {
        return;
    }

    /* Record RSSI */
    BleDatabase_record_rssi(self->database, &scan_msg.pseudo, dev, scan_msg.rssi);

    /* Check device state is idle */
    if(BleDatabase_get_state(self->database, dev) != BleDevice_stateIDLE)
    {
        return;
    }

    /* Check it is time to read the payload */
    if(BleDatabase_payload_needs_read(self->database, dev) == 0)
    {
        return;
    }

    /* Update the device state in the DB */
    BleDatabase_set_state(self->database, dev, BleDevice_stateCONNECTING);

    LOG_DBG("-- Processing: " BleAddr_printStr() " at: " BleAddr_printStr() " --", 
        BleAddr_printParams(&scan_msg.pseudo), BleAddr_printParams(&scan_msg.addr));

    /* Attempt to start the read */
    err = BleReader_read_payload(&self->reader, &scan_msg.addr, &scan_msg.pseudo);

    /* Error check */
    if(err)
    {
        /* Update the device state in the DB */
        BleDatabase_set_state(self->database, dev, BleDevice_stateIDLE);
    }
}

int BleSensor_start(BleSensor_t * self)
{
    /* Start the scanner */
    BleScanner_start(&self->scanner);
    /* Start the advertiser */
    BleAdvertiser_start(&self->advertiser);
    
    return 0;
}

void BleSensor_update_payload(BleSensor_t * self, Data_t * payload)
{
    BleTransmitter_set_payload(&self->transmitter, payload);
}