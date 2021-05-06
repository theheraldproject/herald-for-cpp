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
    err = BleReader_init(&self->reader, self->payload_process_queue);

    if(err)
    {
        LOG_ERR("Reader init!");
        return -1;
    }

    /* Initialize the transmitter */
    err = BleTransmitter_init(&self->transmitter, self->payload_process_queue);

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
    k_msgq_get(self->payload_process_queue, &msg, K_FOREVER);

    /* Get the device from the DB,
    if a device wrote a payload to us it will be created in the DB */
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
        /* Record the failure */
        BleDatabase_payload_not_read(self->database, dev, msg.status);
        return;
    }

    /* Create the data structure */
    data.size = msg.payload_sz;
    data.data = msg.payload;

    /* Record the payload */
    BleDatabase_read_payload(self->database, &msg.pseudo, dev, &data);
}

void BleSensor_read_payloads(BleSensor_t * self)
{
    struct payload_req_msg msg;
    BleDevice_t * dev;
    int err;

    /* Wait for payload read requests */
    k_msgq_get(self->payload_read_queue, &msg, K_FOREVER);

    /* Attempt to start the read */
    err = BleReader_read_payload(&self->reader, &msg.addr, &msg.pseudo);

    /* Error check */
    if(err)
    {
        /* Get the device from the DB, should not have to create a device here */
        dev = BleDatabase_find_create_device(self->database, &msg.pseudo);
        if(dev == NULL)
        {
            LOG_ERR("Find to mark not read!");
            return;
        }
        /* Update device */
        BleDatabase_payload_not_read(self->database, dev, BleErr_SYSTEM);
    }
}

void BleSensor_process_scan(BleSensor_t * self)
{
    BleDevice_t * dev;
    struct scan_results_message scan_msg;
    struct payload_req_msg payload_req_msg;
    int err;

    assert(self);
    
    /* Wait for scan result */
    err = k_msgq_get(self->scan_queue, &scan_msg, K_FOREVER);

    /* Check if a scan result message was received */
    if(err)
    {
        return;
    }

    /* Run the DB delegate */
    BleDatabase_rssi_found(self->database, &scan_msg.pseudo, scan_msg.rssi);

    /* Check if it is known not a herald device,
    no need to include in DB or proccess it */
    if(scan_msg.could_be_herald == 0)
    {
        return;
    }

    /* Add/find in DB */
    dev = BleDatabase_find_create_device(self->database, &scan_msg.pseudo);

    if(dev == NULL)
    {
        LOG_ERR("DB find create!");
        return;
    }

    /* Record the scan */
    BleDatabase_scanned(self->database, dev);

    /* Check it is time to read the payload, this also updates state */
    if(BleDatabase_payload_should_read(self->database, dev) == 0)
    {
        return;
    }

    LOG_DBG("-- Processing: " BleAddr_printStr() " at: " BleAddr_printStr() " --", 
        BleAddr_printParams(&scan_msg.pseudo), BleAddr_printParams(&scan_msg.addr));
    
    BleDatabase_payload_start_reading(self->database, dev);

    /* Add to the payload request queue */
    BleAddress_copy(&payload_req_msg.addr, &scan_msg.addr);
    BleAddress_copy(&payload_req_msg.pseudo, &scan_msg.pseudo);

    /* Send the request message */
    err = k_msgq_put(self->payload_read_queue, &payload_req_msg, K_SECONDS(10));

    if(err)
    {
        LOG_ERR("Could not add payload request!");
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