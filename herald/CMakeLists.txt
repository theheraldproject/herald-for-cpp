cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs
)

set(HEADERS 
	include/herald.h
	include/context.h
	include/device.h
  include/default_sensor_delegate.h
  include/sensor_array.h 
  include/sensor_delegate.h
  include/sensor.h
  include/zephyr_context.h
  include/ble/ble_concrete.h
  include/ble/ble_database_delegate.h
  include/ble/ble_database.h
  include/ble/ble_device_delegate.h
  include/ble/ble_device.h
  include/ble/ble_receiver.h
  include/ble/ble_sensor.h
  include/ble/ble_sensor_configuration.h
  include/ble/ble_transmitter.h
  include/ble/ble_tx_power.h
  include/ble/bluetooth_state_manager.h
  include/ble/bluetooth_state_manager_delegate.h
  include/data/sensor_logger.h
  include/datatype/base64_string.h 
  include/datatype/bluetooth_state.h
  include/datatype/data.h
  include/datatype/date.h
  include/datatype/encounter.h
  include/datatype/error_code.h
  include/datatype/immediate_send_data.h
  include/datatype/location_reference.h
  include/datatype/location.h
  include/datatype/payload_data.h
  include/datatype/payload_sharing_data.h
  include/datatype/payload_timestamp.h
  include/datatype/placename_location_reference.h
  include/datatype/proximity.h
  include/datatype/pseudo_device_address.h
  include/datatype/randomness.h
  include/datatype/rssi.h
  include/datatype/sensor_state.h
  include/datatype/sensor_type.h
  include/datatype/signal_characteristic_data.h
  include/datatype/target_identifier.h
  include/datatype/time_interval.h
  include/datatype/wgs84.h
  include/payload/payload_data_supplier.h
  include/payload/beacon/beacon_payload_data_supplier.h
  include/payload/extended/extended_data.h

)

add_library(herald 
	${HEADERS}
  src/ble/windows/concrete_ble_sensor.cpp
  src/data/sensor_logger.cpp
	src/datatype/base64_string.cpp
	src/datatype/data.cpp
	src/datatype/date.cpp
	src/datatype/encounter.cpp
	src/datatype/immediate_send_data.cpp
	src/datatype/location.cpp
	src/datatype/payload_data.cpp
	src/datatype/pseudo_device_address.cpp
	src/datatype/rssi.cpp
	src/datatype/signal_characteristic_data.cpp
	src/datatype/target_identifier.cpp
	src/datatype/time_interval.cpp
	src/datatype/uuid.cpp
	src/payload/beacon/beacon_payload_data_supplier.cpp
	src/payload/extended/extended_data.cpp
  src/default_sensor_delegate.cpp
  src/context.cpp
  src/zephyr_context.cpp
  src/sensor_array.cpp
)
set_target_properties(herald PROPERTIES PUBLIC_HEADER "${HEADERS}")

include_directories(../fmt/include)

target_include_directories(herald 
	PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

#target_compile_features(herald PRIVATE cxx_std_17)

# NB: This is here to ensure binaries that link us also link stdc++fs for non-Apple targets
# https://github.com/OpenRCT2/OpenRCT2/pull/10522
if(NOT (APPLE OR MSVC) )
#target_link_libraries(herald PRIVATE stdc++fs)
endif(NOT (APPLE OR MSVC) )

install(TARGETS herald 
    EXPORT herald
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/herald
)

install(EXPORT herald 
	NAMESPACE herald::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/herald
)
