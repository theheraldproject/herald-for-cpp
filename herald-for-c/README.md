# Herald for C

A version of herald written in C, used as a near term working alternative to herald for CPP

## Limitations from the CPP version

didDetect callback only gets called on devices that have potential to be herald, 
ie devices advertising the Herald service UUID and Apple devices that pass the Apple filter,
this is done to save space in the BLE database. The detection of every new device can be implemented by the user.

There is no option for a didMeasure with payload callback. This is done to save space in the BLE database. The database stores 
the ID or pseudo ID of the device and passes that via the didRead callback. It is up to the end user to store the ID
of the device and associate the ID with may RSSI values and payloads. This uses less memory in RAM but may use more
non-volatile memory if a payload is being stored multiple times under multiple IDs.

The callbacks are not passing a reference or pointer to the BleDevice. It is up to the end user to keep track of any required info.
