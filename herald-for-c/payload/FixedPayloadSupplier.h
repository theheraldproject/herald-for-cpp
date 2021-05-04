/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __FIXED_PAYLOAD_SUPPLIER__
#define __FIXED_PAYLOAD_SUPPLIER__

#include "payload/PayloadInterface.h"
#include "payload/PayloadSupplier.h"

#define FixedPayloadSupplierPAYLOAD_ID 0x08
#define FixedPayloadSupplierPAYLOAD_SIZE 97

#define FixedPayloadSupplier_INIT(_countryCode, _stateCode, _clientId) \
{ \
    (_countryCode), (_stateCode), (_clientId) \
}

#define FixedPayloadSupplier_PAYLOAD_SUPPLIER_DEF(_self) \
    PayloadSupplier_INIT(&FixedPayloadSupplier_interface, (_self));

typedef struct fixed_payload_supplier_data_s
{
    uint16_t countryCode;
    uint16_t stateCode;
    uint64_t clientId;
}
FixedPayloadSupplier_data_t;

static inline void FixedPayloadSupplier_setClientId(FixedPayloadSupplier_data_t * self, uint64_t id)
{
    self->clientId = id;
}

extern const PayloadInterface_t FixedPayloadSupplier_interface;

#endif /* __FIXED_PAYLOAD_SUPPLIER__ */