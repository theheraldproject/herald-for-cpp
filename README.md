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

Copyright 2020 VMware, Inc.

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache2.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)

See LICENSE.txt and NOTICE.txt for details.

## Implementation differences

Some thin wrappers were not required in the C++ version compared
to the Java and Swift versions as the base primitives were already
accesible in C++17. These include:-

- uint16 (std::uint16)
- uint8 (std::uint8)
- float16 (std::float)
- tuple<A,B> (std::tuple<A,B>)
- triple<A,B,C> (std::tuple<A,B,C>)
- callback (std::bind return val / lambda)
- BLETimer -> Not implemented. Android specific (Android has a bug in its Timer implementation that makes they awake unreliably)
- Context -> Specific to an application's Context on Android, implemented here in case some platforms have similar requirements

We've also added some classes to make porting easier:-

- herald::datatype::uuid - high level UUID interface with just the functionality required by Herald and no more.

## Implementation details

Any trivial wrapper classes have been implemented 
as structs.

Any Interfaces from Java and Swift have been implemented as
pure virtual base classes. These shall always be referred to via
std::shared_ptr wrappers to prevent memory leaks.

The API has been implemented using the PIMPL idiom throughout
to hide internal members and implementation details even when
the classes appear trivial. This is to ensure binary ABI compatibility
in future, which is very useful in compiled c++ code for long lived
code bases that depend upon Herald so they don't require a recompile.

This code base implements Bluetooth Low Energy (BLe) and BLe Mesh
implementations on native Windows and Zephyr/nRF Connect as standard.

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
