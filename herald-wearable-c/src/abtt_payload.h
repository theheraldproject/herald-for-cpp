/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __ABTT_PAYLOAD_H__
#define __ABTT_PAYLOAD_H__

#include <stdint.h>

#define AbttPayload_HERALD_PROTOCOL_VERSION 0x91

int AbttPayload_parse(uint8_t * data, uint16_t len);

#endif /* __ABTT_PAYLOAD_H__ */