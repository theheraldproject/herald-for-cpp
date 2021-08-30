/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/BleOsTransmitter.h"
#include "ble/BleTransmitter.h"
#include "logger/herald_logger.h"

#include "ble/BleReader.h" // Needed for payload message
#include "ble/BleErrCodes.h"

#define prvNUM_INCOMING_PAYLOADS CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE

static int prv_allow_connection_cb(void * module, const BleAddress_t * addr)
{
    /* Currently anyone can connect */
    return 0;
}

static struct ble_tx_incoming_messages * 
    prv_get_free_payload_mem(BleTransmitter_t * self, const BleAddress_t * addr)
{
    size_t i;
    for(i=0;i<prvNUM_INCOMING_PAYLOADS;i++)
    {
        if(self->incoming_msgs[i].used != 0)
        {
            continue;
        }

        /* Found free memory! Set used */
        self->incoming_msgs[i].used = 1;
        /* Reset size */
        self->incoming_msgs[i].current_payload_sz = 0;
        /* Set the address */
        BleAddress_copy(&self->incoming_msgs[i].addr, addr);

        /* Return it */
        return &self->incoming_msgs[i];
    }
    return NULL;
}

static struct ble_tx_incoming_messages * 
    prv_find_payload_mem(BleTransmitter_t * self, const BleAddress_t * addr)
{
    size_t i;
    for(i=0;i<prvNUM_INCOMING_PAYLOADS;i++)
    {
        /* Only looking for used blocks */
        if(self->incoming_msgs[i].used == 0)
        {
            continue;
        }

        /* Compare the addresses */
        if(BleAddress_cmp(addr, &self->incoming_msgs[i].addr) != 0)
        {
            continue;
        }

        /* Fount it! */
        return &self->incoming_msgs[i];
    }

    /* Was not found */
    return NULL;
}

static int prv_get_payload_cb(void * module, const BleAddress_t * addr, Data_t * payload)
{
    BleTransmitter_t * self = (BleTransmitter_t*) module;
    assert(self);

    /* Give the data */
    payload->data = self->payload;
    payload->size = self->payload_size;

    LOG_DBG("Read by " BleAddr_printStr(), BleAddr_printParams(addr));

    /* TODO Mutex should be used to make sure payload does not
    change while it is being read */

    /* return success */
    return 0;
}

static void prv_send_payload_message(BleTransmitter_t * self, const BleAddress_t * addr, 
    uint8_t * data, size_t sz)
{
    struct payload_msg msg;
    int err;

    /* Set the status */
    msg.status = BleErr_OK;

    /* Copy the address, using address for now */
    BleAddress_copy(&msg.pseudo, addr);

    /* Copy the data,
        we know the size of the two arrays are the same, so no need to check size */
    memcpy(msg.payload, data, sz);
    /* Set the size */
    msg.payload_sz = sz;

    /* Send the message */
    err = k_msgq_put(self->payload_queue, &msg, K_NO_WAIT);

    if(err)
    {
        LOG_ERR("Send payload msg from TX!");
    }
}

static int prv_received_payload_cb(void * module, const BleAddress_t * addr, Data_t * data, int offset)
{
    BleTransmitter_t * self = (BleTransmitter_t*) module;
    assert(self);

    struct ble_tx_incoming_messages * mem;

    /* See if we are already reading */
    mem = prv_find_payload_mem(self, addr);

    /* Check if the write is done */
    if(offset < 0)
    {
        if(mem == NULL)
        {
            /* This will be a common case if a device connects without writing a payload */
            //LOG_ERR("Payload mem does not exist");
            LOG_DBG("No payload written");
            return -1;
        }

        if(data->size != 0)
        {
            LOG_WRN("TX payload data set!");
        }

        /* Attempt to send the payload to the queue */
        prv_send_payload_message(self, addr, mem->payload, mem->current_payload_sz);
        /* Free the memory */
        mem->used = 0;
        return 0;
    }

    if(mem == NULL)
    {
        LOG_DBG("Get new payload");
        /* Allocate new memory for this address */
        mem = prv_get_free_payload_mem(self, addr);
    }

    if(mem == NULL)
    {
        LOG_ERR("Could not get TX incoming mem!");
        return -1;
    }

    /* Check the size */
    if(offset + data->size > CONFIG_HERALD_MAX_PAYLOAD_SIZE)
    {
        LOG_WRN("TX incoming payload too big. Size requested (%u)", offset+data->size);
        return -1;
    }

    /* Copy the data */
    memcpy(&mem->payload[offset], data->data, data->size);

    /* Update size if needed */
    if(mem->current_payload_sz < offset + data->size)
    {
        mem->current_payload_sz = offset + data->size;
    }

    return 0;
}

int BleTransmitter_init(BleTransmitter_t * self, struct k_msgq * payload_queue)
{
    /* Initialize the payload mutex */
    if(k_mutex_init(&self->payload_mutex) != 0)
    {
        LOG_ERR("Mutex init!");
        return -1;
    }

    /* Set the payload queue */
    self->payload_queue = payload_queue;

    /* Clear the message memory */
    memset(self->incoming_msgs, 0, 
        sizeof(struct ble_tx_incoming_messages) *
            CONFIG_HERALD_MAX_INCOMING_CONNECTIONS_AT_ONCE);

    return BleOsTransmitter_init((void*) self, prv_allow_connection_cb,
        prv_get_payload_cb, prv_received_payload_cb);
}

/**
 * \brief 
 * 
 * \param self 
 * \param payload 
 * \return 0 for success 
 */
int BleTransmitter_set_payload(BleTransmitter_t * self, Data_t * payload)
{
    k_mutex_lock(&self->payload_mutex, K_FOREVER);

    /* Set the size */
    self->payload_size = payload->size;
    /* Copy the data */
    memcpy(self->payload, payload->data, payload->size);

    k_mutex_unlock(&self->payload_mutex);
    return 0;
}