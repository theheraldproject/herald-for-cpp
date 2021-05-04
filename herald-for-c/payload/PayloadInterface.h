/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __PAYLOAD_INTERFACE__
#define __PAYLOAD_INTERFACE__

#include "data_type/DataTypes.h"

#define PayloadInterface_INIT(_create, _parse) \
{ \
 (_create), (_parse) \
}

/**
 * \brief The interface to create a payload for the given timestamp
 * 
 * \param self
 * \param timestamp The timestamp
 * \param data The data area to write the payload to
 * 
 * \return the size of the payload, negative for error
 */
typedef int (*PayloadInterface_createPayload_t)(const void * self, Timestamp_t timeStamp,
    Data_t * data);

/**
 * The interface to parse payload data
 */
typedef int (*PayloadInterface_parseData_t)(const void * self, Data_t * data);

typedef struct payload_interface_s
{
    PayloadInterface_createPayload_t createPaylaodCb;
    PayloadInterface_parseData_t parseDataCb;
}
PayloadInterface_t;

static inline int PayloadInterface_parse(const PayloadInterface_t * self, const void * module, Data_t * data)
{
    assert(self);
    assert(self->parseDataCb);

    return self->parseDataCb(module, data);
}

static inline int PayloadInterface_createPayload(const PayloadInterface_t * self, const void * module,
    Timestamp_t timestamp, Data_t * data)
{
    assert(self);
    assert(self->createPaylaodCb);

    return self->createPaylaodCb(module, timestamp, data);
}

#endif /* __PAYLOAD_INTERFACE__ */