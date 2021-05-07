/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <string.h>

#include "abtt_payload.h"
#include "logger/herald_logger.h"
#include "payload/HeraldEnvelope.h"

#define prvEXTENSION_DATA_TX_POWER 0x41
#define prvEXTENSION_DATA_RSSI 0x40
#define prvEXTENSION_DATA_MODEL_C 0x42



struct abtt_payload
{
    uint16_t length;
    uint8_t * data;
}
__attribute__((packed, aligned(1)));


static inline void prv_log_arr(uint8_t * data, size_t len)
{
    size_t i;
    for(i=0; i<len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}


int AbttPayload_parse_extension_data(uint8_t * data, uint16_t len)
{
    uint8_t data_code;
    uint8_t data_len;
    
    if(len < 3)
    {
        LOG_ERR("Extended data len err");
        return -1;
    }

    /* Get data code */
    data_code = *data;
    len-=1;
    data+=1;

    /* Get the data length */
    data_len = *data;
    len-=1;
    data+=1;

    if(data_len > len)
    {
        LOG_ERR("data_len > len. %u > %u", data_len, len);
        return -1;
    }

    switch(data_code)
    {
        case prvEXTENSION_DATA_TX_POWER:
            if(data_len != 2)
            {
                LOG_ERR("TX power len!");
                break;
            }
            LOG_DBG("TX power: %d", (uint16_t)*data);
            break;

        case prvEXTENSION_DATA_RSSI:
            if(data_len != 1)
            {
                LOG_ERR("RSSI power len!");
                break;
            }
            LOG_DBG("RSSI: %d", *data);
            break;

        case prvEXTENSION_DATA_MODEL_C:
            LOG_DBG("Phone model: %.*s", data_len, data);
            break;

        default:
            LOG_ERR("Unknown extension data code: 0x%02X", data_code);
            return -1;
            break;
    }

    return data_len + 2;
}

int AbttPayload_parse(uint8_t * data, uint16_t len)
{
    struct herald_envelope * header;
    uint16_t payload_length;
    uint16_t temp_id_len;
    uint8_t * temp_id_data;

    /* Check length is large enough for header */
    if(len < HeraldEnvelopeSIZE)
    {
        LOG_ERR("Length not long enough for header!");
        return -1;
    }

    /* Get the header */
    header = (struct herald_envelope*) data;
    len -= HeraldEnvelopeSIZE;
    data += HeraldEnvelopeSIZE;

    LOG_DBG("--Herald header--");
    HeraldEnvelope_log(header);

    /* Check there is room for the length */
    if(len < 2)
    {
        LOG_DBG("Cannot read payload length");
        return -1;
    }

    /* Get the payload length */
    memcpy(&payload_length, data, 2);
    data += 2;
    len -= 2;


    /* Check the payload length */
    if(payload_length != len)
    {
        if(payload_length > len)
        {
            LOG_ERR("Payload len > data length. %u > %u", payload_length, len);
            return -1;
        }
        LOG_WRN("Extra data!. data len > payload len. %u > %u. Continuing anyway...", len, payload_length);
    }

    LOG_DBG("----ABTT Payload START----");

    /* Note: Only using payload_length from here on, len is not used */

    /* Check we have room for TempID length */
    if(payload_length  < 2)
    {
        LOG_ERR("Cannot get temp ID len");
        return -1;
    }

    /* Get the temp ID length */
    memcpy(&temp_id_len, data, 2);
    data+=2;
    payload_length -= 2;

    LOG_DBG("TempID len: %u", temp_id_len);

    /* Check tempID len */
    if(payload_length < temp_id_len)
    {
        LOG_ERR("payload len < tempID len. %u < %u", payload_length, temp_id_len);
        return -1;
    }

    /* Get the tempID data */
    temp_id_data = data;
    data += temp_id_len;
    payload_length -= temp_id_len;

    LOG_DBG("TempID: ");
    prv_log_arr(temp_id_data, temp_id_len);

    LOG_DBG("Data sz: %u, Data: ", payload_length);
    prv_log_arr(data, payload_length);

    int ret_sz;

    while(payload_length > 0)
    {
        ret_sz = AbttPayload_parse_extension_data(data, payload_length);
        if(ret_sz <= 0)
        {
            return -1;
        }
        /* Update data and length,
        The parse function makes sure ret_sz is not longer then payload_length */
        data += ret_sz;
        payload_length -= ret_sz;
    }

    LOG_DBG("----ABTT Payload END----");
    
    return 0;
}
