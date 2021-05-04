/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __PAYLOAD_SUPPLIER__
#define __PAYLOAD_SUPPLIER__

#include "payload/PayloadInterface.h"

#define PayloadSupplier_INIT(_interface, _payloadSupplier) \
{ \
    (_interface), (_payloadSupplier) \
}

typedef struct payload_supplier_s
{
    const PayloadInterface_t * interface;
    const void * payloadSupplier;
}
PayloadSupplier_t;

static inline int PayloadSupplier_createPayload(PayloadSupplier_t * self, Timestamp_t timestamp, Data_t * data)
{
    assert(self);
    
    return PayloadInterface_createPayload(self->interface, self->payloadSupplier, timestamp, data);
}

static inline int PayloadSupplier_parse(PayloadSupplier_t * self, Data_t * data)
{
    assert(self);
    return PayloadInterface_parse(self->interface, self->payloadSupplier, data);
}


#endif /* __PAYLOAD_SUPPLIER__ */