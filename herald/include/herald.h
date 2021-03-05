//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

// FORCE CORRECT UINT32_t TYPES
#include <cstdint>

// Now include STDLIB extensions
#include "herald/datatype/stdlib.h"

/// Main Herald library include for C++ Native platforms
// Convenience include file

// Root namespace
#include "herald/context.h"
#include "herald/default_sensor_delegate.h"
#include "herald/device.h"
#include "herald/sensor_array.h"
#include "herald/sensor_delegate.h"
#include "herald/sensor.h"

#ifdef __ZEPHYR__
#include "herald/zephyr_context.h"
#endif

// Datatype namespace
#include "herald/datatype/base64_string.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/datatype/data.h"
#include "herald/datatype/date.h"
#include "herald/datatype/encounter.h"
#include "herald/datatype/error_code.h"
#include "herald/datatype/immediate_send_data.h"
#include "herald/datatype/location_reference.h"
#include "herald/datatype/location.h"
#include "herald/datatype/payload_data.h"
#include "herald/datatype/payload_sharing_data.h"
#include "herald/datatype/payload_timestamp.h"
#include "herald/datatype/placename_location_reference.h"
#include "herald/datatype/proximity.h"
#include "herald/datatype/randomness.h"
#include "herald/datatype/rssi.h"
#include "herald/datatype/sensor_state.h"
#include "herald/datatype/sensor_type.h"
#include "herald/datatype/signal_characteristic_data.h"
#include "herald/datatype/target_identifier.h"
#include "herald/datatype/time_interval.h"
#include "herald/datatype/uuid.h"
#include "herald/datatype/wgs84.h"

// data namespace
#include "herald/data/contact_log.h"
#include "herald/data/payload_data_formatter.h"
#include "herald/data/sensor_logger.h"

// engine namespace
#include "herald/engine/activities.h"
#include "herald/engine/coordinator.h"

// ble namespace
#include "herald/ble/ble_coordinator.h"
#include "herald/ble/ble_database_delegate.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_device_delegate.h"
#include "herald/ble/ble_device.h"
#include "herald/ble/ble_mac_address.h"
#include "herald/ble/ble_protocols.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/ble_sensor_configuration.h"
#include "herald/ble/ble_transmitter.h"
#include "herald/ble/ble_tx_power.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/ble/bluetooth_state_manager_delegate.h"

#include "herald/ble/filter/ble_advert_types.h"
#include "herald/ble/filter/ble_advert_parser.h"

#include "herald/ble/ble_concrete.h"

// analysis namespace

// payload namespace
#include "herald/payload/payload_data_supplier.h"

#include "herald/payload/beacon/beacon_payload_data_supplier.h"
#include "herald/payload/fixed/fixed_payload_data_supplier.h"
#include "herald/payload/simple/contact_identifier.h"
#include "herald/payload/simple/contact_key.h"
#include "herald/payload/simple/contact_key_seed.h"
#include "herald/payload/simple/f.h"
#include "herald/payload/simple/k.h"
#include "herald/payload/simple/matching_key.h"
#include "herald/payload/simple/secret_key.h"
#include "herald/payload/simple/simple_payload_data_supplier.h"
#include "herald/payload/extended/extended_data.h"

// service namespace
