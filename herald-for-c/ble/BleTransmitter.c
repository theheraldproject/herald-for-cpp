/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "ble/BleOsTransmitter.h"
#include "ble/BleTransmitter.h"
#include "logger/herald_logger.h"

static int prv_allow_connection_cb(void * module, const BleAddress_t * addr)
{
    return 0;
}

static int prv_get_payload_cb(void * module, const BleAddress_t * addr, Data_t * payload)
{
    BleTransmitter_t * self = (BleTransmitter_t*) module;
    assert(self);

    /* Give the data */
    payload->data = self->payload;
    payload->size = self->payload_size;

    LOG_DBG("Read by " BleAddr_printStr(), BleAddr_printParams(addr));

    //TODO: Use the mutex somehow

    /* return success */
    return 0;
}

static int prv_received_payload_cb(void * module, const BleAddress_t * addr)
{
    LOG_WRN("Received payload to write characteristic! Not recording!");
    /* TODO: Store chunks and send to payload queue one done */
    return 0;
}

int BleTransmitter_init(BleTransmitter_t * self)
{
    /* Initialize the payload mutex */
    if(k_mutex_init(&self->payload_mutex) != 0)
    {
        LOG_ERR("Mutex init!");
        return -1;
    }

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