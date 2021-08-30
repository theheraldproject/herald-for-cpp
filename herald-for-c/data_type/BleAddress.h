/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_ADDRESS_H__
#define __BLE_ADDRESS_H__

#include <string.h>
#include <stdint.h>

/** 
 * Defines and initializes a BLE address structure 
 * \param _addr The address array 
 */
#define BleAddress_DEF(_a,_b,_c,_d,_e,_f) \
{ \
    {(_a),(_b),(_c),(_d),(_e),(_f)} \
}

#define BleAddr_printParams(_p_addr) \
    (_p_addr)->val[5], (_p_addr)->val[4], (_p_addr)->val[3], (_p_addr)->val[2],  \
    (_p_addr)->val[1],(_p_addr)->val[0]

#define BleAddr_printStr() "%02X:%02X:%02X:%02X:%02X:%02X"

#define BleAddr_printf(_p_addr) \
    printf("ADDR:" BleAddr_printStr() "\r\n", BleAddr_printParams(_p_addr))

/** Structure to hold a MAC address in */
typedef struct ble_address_s
{
    uint8_t val[6];
}
BleAddress_t;

static inline void BleAddress_copy(BleAddress_t * to, const BleAddress_t * from)
{
    memcpy(to, from, sizeof(BleAddress_t));
}

/**
 * @brief Compare two addresses
 * 
 * LSB is checked first
 * 
 * @return  positive if a > b
 *          negative if a < b
 *          zero     if a = b
 */
static inline int BleAddress_cmp(const BleAddress_t * a, const BleAddress_t * b)
{
    int i;

    for(i=0; i<6; i++)
    {
        if(a->val[i] == b->val[i])
        {
            continue;
        }
        
        if(a->val[i] > b->val[i])
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    
    return 0;
}

#endif /* __BLE_ADDRESS_H__ */