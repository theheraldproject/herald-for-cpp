# /*
#  * Copyright 2020-2021 Herald Project Contributors
#  * SPDX-License-Identifier: Apache-2.0
#  * 
#  */

set(HERALD_SOURCES
    "${HERALD_BASE}/ble/zephyr/os_device.c"
    "${HERALD_BASE}/ble/zephyr/os_scanner.c"
    "${HERALD_BASE}/ble/zephyr/os_advertiser.c"
    "${HERALD_BASE}/ble/zephyr/os_reader.c"
    "${HERALD_BASE}/ble/zephyr/os_transmitter.c"

    "${HERALD_BASE}/ble/zephyr/zephyr_scanner.c"
    "${HERALD_BASE}/ble/zephyr/zephyr_advertiser.c"
    "${HERALD_BASE}/ble/zephyr/zephyr_gatt.c"
    "${HERALD_BASE}/ble/zephyr/zephyr_connections.c"
    "${HERALD_BASE}/ble/zephyr/zephyr_con_manager.c"
    "${HERALD_BASE}/ble/zephyr/zephyr_in_cons.c"


    "${HERALD_BASE}/ble/zephyr/zephyr_herald_uuid.c"

    "${HERALD_BASE}/ble/BleScanner.c"
    "${HERALD_BASE}/ble/BleReader.c"
    "${HERALD_BASE}/ble/BleTransmitter.c"

    "${HERALD_BASE}/ble/BleDevice.c"

    "${HERALD_BASE}/sensor/BleSensor.c"
    "${HERALD_BASE}/sensor/BleSensor_delegate.c"

    "${HERALD_BASE}/database/BleDatabase.c"
    "${HERALD_BASE}/database/BleDbArray.c"

    "${HERALD_BASE}/sys/Timestamp.c"

    "${HERALD_BASE}/payload/FixedPayloadSupplier.c"
)

# Add Herald configuration
# Set config path
set(BASE_CONFIG "${HERALD_BASE}/config")

# Add the base config
set(OVERLAY_CONFIG "${BASE_CONFIG}/base.conf")

# Add config based on CMAKE build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    # Add debug configuration
    set(OVERLAY_CONFIG ${OVERLAY_CONFIG} "${BASE_CONFIG}/debug.conf")
else()
    # Add release configuration
    set(OVERLAY_CONFIG ${OVERLAY_CONFIG} "${BASE_CONFIG}/release.conf")
endif()

# Include the zephyr package, this creates the `app` target
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Add the include directory
target_include_directories(app PUBLIC include ${HERALD_BASE} ${HERALD_BASE}/include)

# Add the Herald sources
target_sources(app PRIVATE ${HERALD_SOURCES})