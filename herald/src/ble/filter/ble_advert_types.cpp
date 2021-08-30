//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/filter/ble_advert_types.h"

#include <string>
#include <unordered_map>

namespace herald {
namespace ble {
namespace filter {
  

static std::unordered_map<std::string,BLEAdvertSegmentType> advertSegmentStringsToTypes {
  {"unknown",BLEAdvertSegmentType::unknown}
};



BLEAdvertSegmentType
typeFor(int code)
{
  switch (code) {
    case to_integral(BLEAdvertSegmentType::flags):
      return BLEAdvertSegmentType::flags;
    case to_integral(BLEAdvertSegmentType::bigInfo):
      return BLEAdvertSegmentType::bigInfo;
    case to_integral(BLEAdvertSegmentType::broadcastCode):
      return BLEAdvertSegmentType::broadcastCode;
    case to_integral(BLEAdvertSegmentType::deviceClass):
      return BLEAdvertSegmentType::deviceClass;
    case to_integral(BLEAdvertSegmentType::deviceID):
      return BLEAdvertSegmentType::deviceID;
    case to_integral(BLEAdvertSegmentType::deviceNameComplete):
      return BLEAdvertSegmentType::deviceNameComplete;
    case to_integral(BLEAdvertSegmentType::deviceNameShortened):
      return BLEAdvertSegmentType::deviceNameShortened;
    case to_integral(BLEAdvertSegmentType::manufacturerData):
      return BLEAdvertSegmentType::manufacturerData;
    case to_integral(BLEAdvertSegmentType::meshBeacon):
      return BLEAdvertSegmentType::meshBeacon;
    case to_integral(BLEAdvertSegmentType::meshMessage):
      return BLEAdvertSegmentType::meshMessage;
    case to_integral(BLEAdvertSegmentType::serviceUUID128CompleteList):
      return BLEAdvertSegmentType::serviceUUID128CompleteList;
    case to_integral(BLEAdvertSegmentType::serviceUUID128IncompleteList):
      return BLEAdvertSegmentType::serviceUUID128IncompleteList;
    case to_integral(BLEAdvertSegmentType::serviceUUID16CompleteList):
      return BLEAdvertSegmentType::serviceUUID16CompleteList;
    case to_integral(BLEAdvertSegmentType::serviceUUID16IncompleteList):
      return BLEAdvertSegmentType::serviceUUID16IncompleteList;
    case to_integral(BLEAdvertSegmentType::serviceUUID32CompleteList):
      return BLEAdvertSegmentType::serviceUUID32CompleteList;
    case to_integral(BLEAdvertSegmentType::serviceUUID32IncompleteList):
      return BLEAdvertSegmentType::serviceUUID32IncompleteList;
    case to_integral(BLEAdvertSegmentType::simplePairingHash):
      return BLEAdvertSegmentType::simplePairingHash;
    case to_integral(BLEAdvertSegmentType::simplePairingRandomiser):
      return BLEAdvertSegmentType::simplePairingRandomiser;
    case to_integral(BLEAdvertSegmentType::txPowerLevel):
      return BLEAdvertSegmentType::txPowerLevel;
    default:
      return BLEAdvertSegmentType::unknown;
  }
}

// BLEAdvertSegmentType typeFor(const std::string& name)
// {
//  // TODO fill this out
//   return BLEAdvertSegmentType::unknown;
// }


}
}
}
