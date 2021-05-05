/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_DEVICE__
#define __BLE_DEVICE__

#include "data_type/DataTypes.h"
#include "logger/herald_logger.h"

/**
 * \defgroup BleDevice BLE Device
 * All info needed for a BLE Device to run
 * There is a circular dependency between BleDeviceDelegate_t and BleDevice_t
 * so they are both in this module
 * \{
 */

/**
 * Type of attributes that can be updated on a BLE device,
 * used for did_update callback
 */
typedef enum ble_device_attribute_e
{
    BleDeviceAttrPERIPHERAL,
    BleDeviceAttrSTATE,
    BleDeviceAttrOS,
    BleDeviceAttrPAYLOAD_DATA,
    BleDeviceAttrRSSI,
    BleDeviceAttrTX_POWER,
    BleDeviceAttrIMMEDIATE_SEND_DATA
}
BleDeviceAttribute_t;

typedef enum ble_device_state_e
{
    BleDevice_stateIDLE,
    BleDevice_stateCONNECTING
}
BleDevice_state_t;

/**
 * The BLE device structure
 * Contains all info needed for a known device
 * 
 */
typedef struct ble_device_s
{
    uint32_t expiryTime;
    /**< The time to that the device will expire and be deleted
     * 
     */

    uint32_t nextRead;
    /**< The next time to read a payload in seconds
     * 
     * Used to determine if a new payload read is needed
     * 
     * If herald is not found or to many connections errors then
     * this will be set for a larger amount of time
     * If payload is read then it will be set for the normal time
     */

    BleDevice_state_t state;
    /**< The current device state
     * Right not just IDLE and CONNECTING
    */

    uint8_t herald_not_found;
    /**<
     * This count's the number of times a device has not had herrald service on it
     * If herald UUID is found then this should be reset to zero
     */
}
BleDevice_t;

static inline void BleDevice_init(BleDevice_t * self)
{
    assert(self);
    self->expiryTime = 0;
    self->nextRead = 0;
    self->state = BleDevice_stateIDLE;
    self->herald_not_found = 0;
}

static inline void BleDevice_scanned(BleDevice_t * self)
{
    assert(self);
    /* Update the expiry time */
    self->expiryTime = Timestamp_now_s() + CONFIG_HERALD_DEVICE_EXPIRY_SEC;
}

/**
 * \brief Call upon a successfull payload read
 * 
 * Updates state and next read time
 * 
 * \param self 
 */
static inline void BleDevice_payload_read_success(BleDevice_t * self)
{
    assert(self);
    if(self->state != BleDevice_stateCONNECTING)
    {
        LOG_WRN("State is not CONNECTING!");
    }

    /* Reset herald not found */
    self->herald_not_found = 0;

    /* Update state */
    self->state = BleDevice_stateIDLE;

    /* Update the next read time */
    self->nextRead = Timestamp_now_s() + CONFIG_HERALD_PAYLOAD_READ_INTERVAL_S;
}

/**
 * \brief Check if the device needs the payload read
 * 
 * \param self 
 * 
 * \return ==0 it does not need to be read
 * \return !=0 it should be read
 */
static inline int BleDevice_payloadShouldRead(const BleDevice_t * self)
{
    assert(self);

    /* Check state */
    if(self->state != BleDevice_stateIDLE)
    {
        return 0;
    }

    /* Just check the timestamp */
    if(self->nextRead > Timestamp_now_s())
    {
        return 0;
    }

    return 1;
}

static inline void BleDevice_startingRead(BleDevice_t * self)
{
    if(self->state != BleDevice_stateIDLE)
    {
        LOG_WRN("State not IDLE when staring read!");
    }
    /* Update state */
    self->state = BleDevice_stateCONNECTING;
}

/**
 * \brief 
 * 
 * \param self 
 * \param current_sec 
 * \return ==0 it is not expired
 */
static inline int BleDevice_isExpired(BleDevice_t * self, uint32_t current_sec)
{
    if(self->expiryTime < current_sec)
    {
        return 1;
    }
    
    return 0;
}

void BleDevice_payload_not_read(BleDevice_t * self, int8_t err);

/** } */

#endif /* __BLE_DEVICE__ */