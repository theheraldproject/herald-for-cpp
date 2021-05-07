/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __HERALD_ENVELOPE_H__
#define __HERALD_ENVELOPE_H__

#include "logger/herald_logger.h"

#define HeraldEnvelopeSIZE sizeof(struct herald_envelope)

struct herald_envelope
{
    uint8_t protocol_version;
    uint16_t country;
    uint16_t state;
}
__attribute__((packed, aligned(1)));


static inline void HeraldEnvelope_log(struct herald_envelope * self)
{
    /* Log */
    LOG_DBG("Protocol version: %02X", self->protocol_version);
    LOG_DBG("Country code: %u", self->country);
    LOG_DBG("State code: %u", self->state);
}

#endif /* __HERALD_ENVELOPE_H__ */