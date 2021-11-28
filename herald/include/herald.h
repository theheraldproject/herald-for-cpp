//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

// PLATFORM SPECIFIC OVERRIDES FIRST
#include "herald/platform.h"

// FORCE CORRECT UINT32_t TYPES
#include <cstdint>

// Now include STDLIB extensions
#include "herald/datatype/stdlib.h"
#include "herald/util/is_valid.h"

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
#include "herald/data/zephyr/zephyr_logging_sink.h"
#include "herald/ble/zephyr/concrete_ble_transmitter.h"
#ifdef CONFIG_BT_SCAN
#include "herald/ble/zephyr/concrete_ble_receiver.h"
#else
#include "herald/ble/default/concrete_ble_receiver.h"
#endif
#include "herald/data/zephyr/flash_exposure_store.h"
#include "herald/data/zephyr/flash_risk_store.h"
#endif

// Datatype namespace
#include "herald/datatype/allocatable_array.h"
#include "herald/datatype/base64_string.h"
#include "herald/datatype/bluetooth_state.h"
#include "herald/datatype/data.h"
#include "herald/datatype/date.h"
#include "herald/datatype/distance.h"
#include "herald/datatype/distribution.h"
#include "herald/datatype/encounter.h"
#include "herald/datatype/error_code.h"
#include "herald/datatype/exposure_risk.h"
#include "herald/datatype/immediate_send_data.h"
#include "herald/datatype/location_reference.h"
#include "herald/datatype/location.h"
#include "herald/datatype/memory_arena.h"
#include "herald/datatype/model.h"
#include "herald/datatype/payload_data.h"
#include "herald/datatype/payload_sharing_data.h"
#include "herald/datatype/payload_timestamp.h"
#include "herald/datatype/placename_location_reference.h"
#include "herald/datatype/proximity.h"
#include "herald/datatype/randomness.h"
//#include "herald/datatype/risk_score.h"
#include "herald/datatype/rssi.h"
#include "herald/datatype/rssi_minute.h"
#include "herald/datatype/sha256.h"
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
#include "herald/data/stdout_logging_sink.h"

// engine namespace
#include "herald/engine/activities.h"
#include "herald/engine/coordinator.h"

// ble namespace
#include "herald/ble/ble.h"
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
#include "herald/ble/ble_concrete_database.h"

// ble utilities (safe on non Zephyr)
#include "herald/ble/zephyr/nordic_uart/nordic_uart_sensor_delegate.h"

// analysis namespace
#include "herald/analysis/aggregates.h"
#include "herald/analysis/distance_conversion.h"
#include "herald/analysis/logging_analysis_delegate.h"
#include "herald/analysis/ranges.h"
#include "herald/analysis/runner.h"
#include "herald/analysis/sampling.h"
#include "herald/analysis/sample_algorithms.h"
#include "herald/analysis/sensor_source.h"

// exposure namespace
//#include "herald/exposure/agent.h"
#include "herald/exposure/exposure_manager.h"

// risk namespace
//#include "herald/risk/risk_manager.h"

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

// utilities
#include "herald/util/byte_array_printer.h"

/// \brief The main Herald Proximity namespace in C++
namespace herald {

}