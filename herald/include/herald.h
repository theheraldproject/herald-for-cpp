//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

// FORCE CORRECT UINT32_t TYPES
#include <cstdint>

/// Main Herald library include for C++ Native platforms
// Convenience include file

// Root namespace
#include "context.h"
#include "default_sensor_delegate.h"
#include "device.h"
#include "sensor_array.h"
#include "sensor_delegate.h"
#include "sensor.h"
#include "zephyr_context.h"

// Datatype namespace
#include "datatype/base64_string.h"
#include "datatype/bluetooth_state.h"
#include "datatype/data.h"
#include "datatype/date.h"
#include "datatype/encounter.h"
#include "datatype/error_code.h"
#include "datatype/immediate_send_data.h"
#include "datatype/location_reference.h"
#include "datatype/location.h"
#include "datatype/payload_data.h"
#include "datatype/payload_sharing_data.h"
#include "datatype/payload_timestamp.h"
#include "datatype/placename_location_reference.h"
#include "datatype/proximity.h"
#include "datatype/pseudo_device_address.h"
#include "datatype/randomness.h"
#include "datatype/rssi.h"
#include "datatype/sensor_state.h"
#include "datatype/sensor_type.h"
#include "datatype/signal_characteristic_data.h"
#include "datatype/target_identifier.h"
#include "datatype/time_interval.h"
#include "datatype/uuid.h"
#include "datatype/wgs84.h"

// data namespace
#include "data/sensor_logger.h"

// ble namespace
#include "ble/ble_database_delegate.h"
#include "ble/ble_database.h"
#include "ble/ble_device_delegate.h"
#include "ble/ble_device.h"
#include "ble/ble_receiver.h"
#include "ble/ble_sensor.h"
#include "ble/ble_sensor_configuration.h"
#include "ble/ble_transmitter.h"
#include "ble/ble_tx_power.h"
#include "ble/bluetooth_state_manager.h"
#include "ble/bluetooth_state_manager_delegate.h"

#include "ble/ble_concrete.h"

// analysis namespace

// payload namespace
#include "payload/payload_data_supplier.h"

#include "payload/beacon/beacon_payload_data_supplier.h"
#include "payload/extended/extended_data.h"

// service namespace
