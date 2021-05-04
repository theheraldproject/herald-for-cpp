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

    "${HERALD_BASE}/sensor/BleSensor.c"
    "${HERALD_BASE}/sensor/BleSensor_delegate.c"

    "${HERALD_BASE}/database/BleDatabase.c"
    "${HERALD_BASE}/database/BleDbArray.c"

    "${HERALD_BASE}/sys/Timestamp.c"

    "${HERALD_BASE}/payload/FixedPayloadSupplier.c"
)