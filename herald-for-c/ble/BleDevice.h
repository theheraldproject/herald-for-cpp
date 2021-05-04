/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_DEVICE__
#define __BLE_DEVICE__

#include "data_type/DataTypes.h"

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
 * Only the BleDatabase should modify these values
 * The BleReceived and BleTransmitter can check the fields
 * with the provided getters
 */
typedef struct ble_device_s
{
    Rssi_t rssi;
    /**< The last RSSI read from a scan of the device */

    Timestamp_t lastScan;
    /**< The last time the device was scanned */

    Timestamp_t lastPayloadRead;
    /**< The last time a payload was read */

    BleDevice_state_t state;
    /**< The current device state */

    uint8_t herald_not_found;
    /**<
     * This count's the number of times a device has not had herrald service on it
     * If herald UUID is found then this should be reset to zero
     */

    uint8_t connection_errors;
    /**< The number of times we attempted connection to the device
     * and received a connection error
     * This is independent of herald_not_found ie we update one or the other
     */
}
BleDevice_t;

static inline void BleDevice_init(BleDevice_t * self)
{
    assert(self);
    self->lastScan = 0;
    self->lastPayloadRead = 0;
    self->state = BleDevice_stateIDLE;
    self->herald_not_found = 0;
    self->connection_errors = 0;
}

static inline void BleDevice_scanned(BleDevice_t * self)
{
    assert(self);
    Timestamp_now(&self->lastScan);
}

static inline void BleDevice_payload_read(BleDevice_t * self)
{
    assert(self);
    Timestamp_now(&self->lastPayloadRead);
}

static inline void BleDevice_herald_not_found(BleDevice_t * self)
{
    assert(self);
    /* Increment herald not found */
    self->herald_not_found++;
}

static inline void BleDevice_herald_found(BleDevice_t * self)
{
    assert(self);
    /* Increment herald not found */
    self->herald_not_found = 0;
}

static inline void BleDevice_connection_error(BleDevice_t * self)
{
    assert(self);
    self->connection_errors++;
}

static inline void BleDevice_connection_success(BleDevice_t * self)
{
    assert(self);
    self->connection_errors = 0;
}

/**
 * \brief Check if the device should be ignored
 * 
 * \param self Self
 * \return != 0 if should connect
 * \return == 0 if should not connect
 */
static inline uint8_t BleDevice_shouldConnect(const BleDevice_t * self)
{
    assert(self);
    /* Check for herald service not found limit reached */
    if(self->herald_not_found >= CONFIG_HERALD_MAX_ATTEMPTS_TO_FIND_SERVICE)
    {
        return 0;
    }

    /* Check for herald service not found limit reached */
    if(self->connection_errors >= CONFIG_HERALD_MAX_CONNECTION_ATTEMPTS)
    {
        return 0;
    }

    return 1;
}

static inline void BleDevice_set_state(BleDevice_t * self, BleDevice_state_t state)
{
    assert(self);
    self->state = state;
}

static inline BleDevice_state_t BleDevice_get_state(BleDevice_t * self)
{
    assert(self);
    return self->state;
}

/**
 * \brief Check if the device needs the payload read
 * 
 * \param self 
 * 
 * \return ==0 it does not need to be read
 * \return !=0 it should be read
 */
static inline int BleDevice_payloadNeedsRead(const BleDevice_t * self)
{
    Timestamp_t interval;
    uint64_t millis;

    assert(self);

    /* Quick check to see if it has ever been read */
    if(self->lastPayloadRead == 0)
    {
        return 1;
    }

    /* Get the time since the last update */
    Timestamp_time_since(&interval, &self->lastPayloadRead);

    /* Convert the timestamp to milliseconds */
    Timestamp_get_ms(&millis, &interval);

    /* Check the length */
    if(millis > CONFIG_HERALD_PAYLOAD_READ_INTERVAL_MS)
    {
        return 1;
    }

    return 0;
}


/** } */

#endif /* __BLE_DEVICE__ */