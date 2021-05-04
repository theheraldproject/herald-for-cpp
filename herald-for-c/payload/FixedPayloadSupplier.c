/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "FixedPayloadSupplier.h"
#include "payload/PayloadInterface.h"
#include "payload/PayloadSupplier.h"
#include "payload/HeraldEnvelope.h"

#define prvPAYLOAD_ID FixedPayloadSupplierPAYLOAD_ID
#define prvPAYLOAD_SIZE FixedPayloadSupplierPAYLOAD_SIZE

int prv_createPayload(const FixedPayloadSupplier_data_t * self, Timestamp_t timeStamp, 
    Data_t * data)
{
    /* Initialize the data */
    if(data->size < prvPAYLOAD_SIZE)
    {
        return -1;
    }

    uint8_t * prv_data_ptr = data->data;

    /* Set the payload ID */
    *prv_data_ptr = prvPAYLOAD_ID;
    prv_data_ptr += 1;

    /* Set the Country code */
    memcpy(prv_data_ptr, &self->countryCode, sizeof(uint16_t));
    prv_data_ptr += sizeof(uint16_t);

    /* Set the state code */
    memcpy(prv_data_ptr, &self->stateCode, sizeof(uint16_t));
    prv_data_ptr += sizeof(uint16_t);

    /* Set the client ID */
    memcpy(prv_data_ptr, &self->clientId, sizeof(uint64_t));
    prv_data_ptr += sizeof(uint64_t);

    return prvPAYLOAD_SIZE;
}

int prv_parseData(const FixedPayloadSupplier_data_t * self, Data_t * data)
{
    struct herald_envelope * env;
    uint8_t * data_ptr = data->data;
    size_t sz = data->size;
    uint64_t * client_id;

    if(sz < HeraldEnvelopeSIZE)
    {
        LOG_WRN("Not large enough for envelope!");
        return -1;
    }

    env = (struct herald_envelope*)data_ptr;
    data_ptr+=HeraldEnvelopeSIZE;
    sz -= HeraldEnvelopeSIZE;

    HeraldEnvelope_log(env);

    if(sz < sizeof(uint64_t))
    {
        LOG_ERR("No CID!");
        return -1;
    }

    client_id = (uint64_t*) data_ptr;
    data_ptr += sizeof(uint64_t);
    sz -= sizeof(uint64_t);

    LOG_DBG("Client ID: %" PRIu64, *client_id);

    size_t i;
    for(i=0;i<sizeof(uint64_t); i++)
    {
        printk("%02x ", ((uint8_t*) client_id)[i] );
    }

    printk("\r\n");
    return 0;
}

/* Public interface */
const PayloadInterface_t FixedPayloadSupplier_interface =
    PayloadInterface_INIT(
        (PayloadInterface_createPayload_t)prv_createPayload,
        (PayloadInterface_parseData_t)prv_parseData);
