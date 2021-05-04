/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "data_type/DataTypes.h"
#include "ble/BleReader.h"
#include "ble/BleOsReader.h"
#include "ble/BleErrCodes.h"

#include "logger/herald_logger.h"

/**
 * \brief Get free memory to read payload to 
 * 
 * \param self 
 * \param addr 
 * \param pseudo 
 * \return == 0 for success
 * \return != 0 for error 
 */
int prv_setup_payload_mem(BleReader_t * self, const BleAddress_t * addr, const BleAddress_t * pseudo)
{
    int i;
    struct ble_payload_readings_s * payload_mem;
    
    for(i=0; i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME; i++)
    {
        if(self->payload_readings[i].used != 0)
        {
            continue;
        }

        payload_mem = &self->payload_readings[i];

        /* Set used */
        payload_mem->used = 1;
        
        /* Copy the address */
        BleAddress_copy(&payload_mem->addr, addr);
        /* Copy the pseudo address */
        BleAddress_copy(&payload_mem->pseudo, pseudo);
        /* Set current reading to zero */
        payload_mem->current_payload_sz = 0;

        return 0;
    }

    return -1;
}

struct ble_payload_readings_s * prv_get_payload_mem(BleReader_t * self, const BleAddress_t * addr)
{
    int i;

    for(i=0; i<CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME; i++)
    {
        /* Only check used memory */
        if(self->payload_readings[i].used == 0)
        {
            continue;
        }

        /* Check this is the memory */
        if(BleAddress_cmp(addr, &self->payload_readings[i].addr) != 0)
        {
            continue;
        }

        return &self->payload_readings[i];
    }

    return NULL;
}

void prv_free_payload_mem(BleReader_t * self, const BleAddress_t * addr)
{
    struct ble_payload_readings_s * payload_mem = prv_get_payload_mem(self, addr);
    
    if(payload_mem == NULL)
    {
        LOG_ERR("Could not free payload memory!");
        return;
    }

    payload_mem->used = 0;
}


/**
 * \brief Send a payload message
 * 
 * \param self 
 * \param pseudo 
 * \param status 
 * \param data 
 * \param sz 
 */
static inline void prv_send_payload_message(BleReader_t * self, const BleAddress_t * pseudo,
    int status, uint8_t * data, size_t sz)
{
    struct payload_msg msg;
    int err;

    /* Set the status */
    msg.status = status;

    /* Copy the address */
    BleAddress_copy(&msg.pseudo, pseudo);

    if(status == 0)
    {
        /* Copy the data,
        we know the size of the two arrays are the same, so no need to check size */
        memcpy(msg.payload, data, sz);
        /* Set the size */
        msg.payload_sz = sz;
    }
    else
    {
        /* Set the size to zero */
        msg.payload_sz = 0;
    }

    /* Send the message */
    err = k_msgq_put(self->payload_queue, &msg, K_NO_WAIT);

    if(err)
    {
        LOG_ERR("Could not send scan result");
    }
}

static void prv_connection_done(void * module, const BleAddress_t * addr)
{
    BleReader_t * self = (BleReader_t*) module;

    assert(self);

    /* Give up the semephore */
    k_sem_give(&self->conn_sem);
}

static int prv_payload_cb(void * module, const BleAddress_t * addr, int status, const Data_t * data)
{
    struct ble_payload_readings_s * payload_mem;
    

    BleReader_t * self = (BleReader_t*) module;

    assert(self);

    /* Get the payload memory */
    payload_mem = prv_get_payload_mem(self, addr);

    /* Error check */
    if(payload_mem == NULL)
    {
        /* This should never be able to happen!
        Not cleaning anything else up here as this situation will never happen in production */
        LOG_ERR("Could not find payload memory to store payload in");
        return -1;
    }

    if(status)
    {
        /* Send the message with the error */
        prv_send_payload_message(self, &payload_mem->pseudo, status, NULL, 0);
        /* Free the payload memory */
        payload_mem->used = 0;
        /* Return same status */
        return status;
    }

    /* Check we are done reading */
    if(data->data == NULL)
    {
        LOG_DBG("Done reading!");
        /* Send message */
        prv_send_payload_message(self, &payload_mem->pseudo, status,
            payload_mem->payload, payload_mem->current_payload_sz);
        /* Free the payload memory */
        payload_mem->used = 0;
        /* Signal to not read any more data */
        return -1;
    }

    /* Check there is room for the new data */
    if(data->size > CONFIG_HERALD_MAX_PAYLOAD_SIZE - payload_mem->current_payload_sz)
    {
        LOG_ERR("Payload to big to fit in allocated memory! Allocated: %u, Requested: %u",
            CONFIG_HERALD_MAX_PAYLOAD_SIZE, data->size + payload_mem->current_payload_sz);
        
        /* Send message with error */
        prv_send_payload_message(self, &payload_mem->pseudo, BleErr_ERR_PAYLOAD_TO_BIG, NULL, 0);
        /* Free the payload memory */
        payload_mem->used = 0;
        /* Signal to not read anymore data */
        return BleErr_STOP_READING;
    }

    /* Copy the data */
    memcpy(payload_mem->payload + payload_mem->current_payload_sz, data->data, data->size);

    /* Increment the size */
    payload_mem->current_payload_sz += data->size;

    return 0;
}

int BleReader_init(BleReader_t * self, struct k_msgq * payload_queue)
{
    int err;

    /* Add payload queue */
    self->payload_queue = payload_queue;

    /* Initialize the connection semaphore */
    err = k_sem_init(&self->conn_sem, CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME,
        CONFIG_HERALD_MAX_PAYLOAD_READ_AT_ONE_TIME);

    if(err)
    {
        LOG_ERR("reader semaphore init!");
        return err;
    }

    /* Initialize the reader and set the callback */
    err = BleOsReader_init((void*) self, prv_payload_cb, prv_connection_done);

    if(err)
    {
        LOG_ERR("BleOsReader_init!");
        return err;
    }

    return 0;
}

int BleReader_read_payload(BleReader_t * self, const BleAddress_t * addr,
    const BleAddress_t * pseudo)
{
    int err;
    LOG_DBG("Ready to connect, wating for sem");

    /* Wait for other connections to finish */
    //err = k_sem_take(&self->conn_sem, K_MSEC(CONFIG_HERALD_MAX_PAYLOAD_READ_TIME_MS));
    err = k_sem_take(&self->conn_sem, K_FOREVER);

    if(err != 0)
    {
        LOG_ERR("Could not get read semaphore, not reading payload");
        return err;
    }

    /* Setup payload read memory */
    err = prv_setup_payload_mem(self, addr, pseudo);

    if(err != 0)
    {
        LOG_ERR("Could not get memory to read payload to");
        return BleErr_ERR_PAYLOAD_TO_BIG;
    }

    LOG_DBG("Starting connection...");

    err = BleOsReader_get_payload(addr);

    if(err)
    {
        LOG_ERR("Error starting payload read (%d)", err);
        /* Give up the semephore */
        k_sem_give(&self->conn_sem);
        /* Free the payload memory */
        prv_free_payload_mem(self, addr);
        return BleErr_ERR_PAYLOAD_TO_BIG;
    }

    return 0;
}
