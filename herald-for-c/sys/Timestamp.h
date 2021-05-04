/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <assert.h>
#include <stdint.h>

/** A time stamp */
typedef uint64_t Timestamp_t;

void Timestamp_now(Timestamp_t * timestamp);

void Timestamp_get_ms(uint64_t * time_ms, Timestamp_t * timestamp);

static inline void Timestamp_now_ms(uint64_t * ms)
{
    Timestamp_t now;
    Timestamp_now(&now);
    Timestamp_get_ms(ms, &now);
}

static inline void Timestamp_time_since(Timestamp_t * interval, const Timestamp_t * oldTime)
{
    assert(oldTime);
    assert(interval);

    Timestamp_t now;
    Timestamp_now(&now);

    *interval = now - *oldTime;
}

#endif /* __TIMESTAMP_H__ */