/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include "sys/Timestamp.h"

void Timestamp_now(Timestamp_t * timestamp)
{
    *timestamp = k_uptime_get();
}

uint32_t Timestamp_now_s(void)
{
    /* Get MS */
    uint64_t now = k_uptime_get();
    
    /* Convert to seconds */
    return now / 1000;
}

void Timestamp_get_ms(uint64_t * time_ms, Timestamp_t * timestamp)
{
    /* The zephyr k_uptime_get records in milliseconds */
    *time_ms = *timestamp;
}