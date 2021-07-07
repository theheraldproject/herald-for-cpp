# Herald for C++ (Native platforms) - Library

This repository contains a Herald Bluetooth Protocol and Payload
set of implementations for native platforms. This includes
Windows 10 desktop and Nordic Semiconductor or other Zephyr RTOS
capable boards. The board used for development is the
Seeed nRF52840MK-USB-Dongle.

This particular code base is the core Herald library. It is
used by all examples, including Wearables (just like the Herald for
iOS and Herald for Android library use case), Bluetooth MESH 
Gateway, and Windows Desktop Herald communication examples.

This implementation was introduced in Herald v1.2.

## License and Copyright

Copyright 2020-2021 Herald Project Contributors

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache2.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)

See LICENSE.txt and NOTICE.txt for details.

## Demonstration apps / libraries

The following apps are available:-

- herald-tests - Herald core C++ API tests
- herald-venue-beacon - Herald Zephyr RTOS based app for Venue beacons as a replacement/supplement for QR code scanning when visiting business, bars, and restaurants. [See the separate README](./herald-venue-beacon/README.md)
- herald-wearable - Herald Zephyr RTOS based app for wearable devices. The equivalent of the herald-for-ios and herald-for-android demo apps for phones
- heraldns and heraldns-cli and heraldns-tests - Not strictly using the Herald API, but used to test epidemiological/scientific theories that may be merged in to herald's core API in future. Command line utility to simulate social mixing analyses and virus spread.

The following are libraries available for your own projects:-

- herald - Core herald API (include as an external library or call herald.cmake directly to statically compile in to your app)

The following are 'coming soon':-

- herald-mesh-proxy - Rather than a beacon that acts on its own, this mesh proxy provides the same functions of a beacon but also value add functionality from having a Bluetooth 5.0 mesh data network
- herald-programmer - Utility to reprogram Herald based beacons / mesh proxies in the field in a more
user friendly way than development tools (E.g. carehomes, field personnel updates, etc.)

## Supported / tested platforms

The Herald team use the C++ API for specific use cases and so only provide support for a specific
subset of use cases. If you'd like to contribute platform support feel free to provide code, tests, documentation and raise a PR.

- Windows 64-bit with CLang 10.0+ for VS community edition 2017
  - We don't currently support the vsc++ compiler
- Zephyr RTOS / arm with arm-none-eabi-gcc 8.3+ for nRF52840, nRF52832
  - We specifically test using the [Nordic nRF Connect SDK Zephyr variant](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/zephyr.html) [External]

Other platforms which may work but which we do not provide direct advice for:-

- Other [Zephyr supported boards](https://docs.zephyrproject.org/latest/boards/index.html) [External]
- Linux Desktops/laptops (yet)
  - Android devices see the separate [herald-for-android](https://github.com/theheraldproject/herald-for-android/) project
- Apple OS X Desktops/laptops (yet)
  - iOS devices see the separate [herald-for-ios](https://github.com/theheraldproject/herald-for-ios) project

## Implementation differences

Some thin wrappers were not required in the C++ version compared
to the Java and Swift versions as the base primitives were already
accesible in C++17. These include:-

- uint64 (std::uint64)
- uint32 (std::uint32)
- uint16 (std::uint16)
- uint8 (std::uint8)
- float16 (std::float)
- strings (std::string)
- tuple<A,B> (std::tuple<A,B>)
- triple<A,B,C> (std::tuple<A,B,C>)
- callback (unused - syncrhonous only, threading abstracted outside of Herald. See Content class for details.)
- BLETimer -> Not implemented. Android specific (Android has a bug in its Timer implementation that makes they awake unreliably)
- Context -> Specific to an application's Context on Android, implemented here in case some platforms have similar requirements. E.g. the ZephyrContext derived class as Zephyr OS has this requirement around Bluetooth state handling.

We've also added some classes to make porting easier. We may fall back in their implementation
C++ files on some platforms where known utility libraries are always present. Current list includes:-

- herald::datatype::uuid - high level UUID interface with just the functionality required by Herald and no more.
- herald::datatype::base64string - encodes and decodes a Data instance as a Base64 string

## Implementation details

Any trivial wrapper classes have been implemented 
as structs.

Any Interfaces from Java and Swift have been implemented as
pure virtual base classes. The code base is being heavily
refactored to use references only and avoid using any pointers,
including smart pointers. This allows us to be able to predict and
restrict memory use at compile time, and provide for maximum memory
safety.

This code base implements Bluetooth Low Energy (BLe)
implementations on Zephyr/nRF Connect as standard.

## What isn't implemented

Any higher level implementation details, such as Mesh gateway payloads
and interconnect logic is within a downstream library project. That
is to keep this set of classes simple and consistent with the iOS
and Android equivalent Herald libraries.

## What is implemented

We do provide a demo Windows application in this repository, and a demo
zephyr serial application. These act as 'Herald consumer / demo devices'
and implement the same basic features as the Android and iOS demo apps.
They are provided to allow us to carry out regression testing on each
version. 

These demo apps are not intended as production ready applications. They
can be used as a reference implementation for any Herald based apps
and devices you wish to create.

## Specific platform limitations

The Herald API cannot use all available modern C++ features due to
some hardware, OS' and libc++ libraries not supporting those functions.
Where possible we always use modern C++17 techniques and STL functions.
On specific platforms we also check for local utility libraries rather
than respecify our own (E.g. base64, random number generation, uuid).

### Zephyr OS limitations

We cannot use dynamic_pointer_cast to cast a std::shared_ptr<DerivedType>
to a std::shared_ptr<BaseType> because this uses RTTI which is not
supported by Zephyr. We use static_pointer_cast instead, but only when we
know the class implementation can only have one definition (i.e. the
one for the current platform). We are moving to remove all use of
smart pointers generally in favour of references.

We also use noexcept rather than throw exceptions for the same reason.

See the [Zephyr C++ limitations](https://docs.zephyrproject.org/latest/reference/kernel/other/cxx_support.html) [External] page for details.
Note that this page is out of date somewhat. The 'new' keyword, for example, is supported in Zephyr although it is very buggy.

## Building with Code Coverage

1. Open Visual Studio Code
1. Perform a CMake build using CLang on Windows in Debug mode
1. Execute this in the build folder on the command line: ```cmake .. -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=ON``` to add code coverage support
1. Open the CMake tools tab in Visual Studio Code
1. Expand 'herald-tests'
1. Run the 'ccov-report' utility