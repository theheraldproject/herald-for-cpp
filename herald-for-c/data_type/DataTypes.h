/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __DATA_TYPES__
#define __DATA_TYPES__

#include <stdint.h>
#include <stddef.h>

#include "sys/Timestamp.h"
#include "data_type/BleAddress.h"

/**
 * Herrald manufacturer ID, big endian
 */
#define HERALD_MANUFACTURER_ID 0xFFFA

/** A data packet that can be sent or received */
typedef struct data_s
{
    size_t size;
    uint8_t * data;
}
Data_t;

typedef int8_t Rssi_t;

#endif
