//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/ble/filter/ble_advert_types.h"
#include "herald/ble/filter/ble_advert_parser.h"

#include <string>
#include <vector>

namespace herald {
namespace ble {
namespace filter {
namespace BLEAdvertParser {

using namespace herald::datatype;

std::vector<BLEAdvertSegment>
extractSegments(const Data& raw, std::size_t offset) noexcept
{
  std::size_t position = offset;
  std::vector<BLEAdvertSegment> segments;
  std::uint8_t segmentLength;
  std::uint8_t segmentType;

  while (position < raw.size()) {
    if ((position + 2) <= raw.size()) {
      segmentLength = 0;
      bool ok = raw.uint8(position++, segmentLength);
      if (ok) {
        segmentType = 0;
        ok = raw.uint8(position++, segmentType);
        if (ok) {
          // Note: Unsupported types are handled as 'unknown'
          // check reported length with actual remaining data length
          if ((position + segmentLength - 1) <= raw.size()) {
            segments.emplace_back(
              typeFor(segmentType), 
              subDataBigEndian(raw, position, segmentLength - 1)
            );
            position += segmentLength - 1;
          } else {
            // error in data length - advance to end
            position = raw.size();
          }
        }
      }
    } else {
      // invalid segment - advance to end
      position = raw.size();
    }
  }

  return segments;
}


// Parse result extraction functions

bool
extractTxPower(const std::vector<BLEAdvertSegment>& segments, std::uint8_t& into) noexcept
{
  // find the txPower code segment in the list
  for (auto segment : segments) {
    if (segment.type == BLEAdvertSegmentType::txPowerLevel && segment.data.size() > 0) {
      bool ok = segment.data.uint8(0, into);
      if (ok) {
        return ok;
      } // else see if theres a non malformed one...
    }
  }
  return false;
}

std::vector<BLEAdvertManufacturerData>
extractManufacturerData(std::vector<BLEAdvertSegment> segments) noexcept
{
  // find the manufacturerData code segment in the list
  std::vector<BLEAdvertManufacturerData> manufacturerData;
  for (BLEAdvertSegment segment : segments) {
    if (segment.type == BLEAdvertSegmentType::manufacturerData) {
      // Ensure that the data area is long enough
      if (segment.data.size() < 2) {
        continue; // there may be a valid segment of same type... Happens for manufacturer data
      }
      // Create a manufacturer data segment
      std::uint16_t intValue = 0;
      bool ok = segment.data.uint16(0, intValue);
      if (ok) {
        manufacturerData.emplace_back(
          intValue,
          subDataBigEndian(segment.data,2,segment.data.size() - 2)
        );
      }
    }
  }
  return manufacturerData;
}

std::vector<Data>
extractHeraldManufacturerData(std::vector<BLEAdvertManufacturerData> manuData) noexcept
{
  std::vector<Data> heraldSegments;
  for (auto manu : manuData) {
    if (manu.manufacturer != to_integral(BLEAdvertManufacturers::heraldUnregistered)) {
      continue;
    }
    heraldSegments.push_back(manu.data);
  }
  return heraldSegments;
}

std::vector<BLEAdvertAppleManufacturerSegment>
extractAppleManufacturerSegments(std::vector<BLEAdvertManufacturerData> manuData) noexcept
{
  std::vector<BLEAdvertAppleManufacturerSegment> appleSegments;
  std::size_t bytePos;
  for (auto manu : manuData) {
    if (manu.manufacturer != to_integral(BLEAdvertManufacturers::apple)) {
      continue;
    }
    bytePos = 0;
    while (bytePos < manu.data.size()) {
      std::uint8_t typeValue = 0;
      bool ok = manu.data.uint8(bytePos, typeValue);
      if (ok) {
        // "01" marks legacy service UUID encoding without length data
        if (typeValue == 0x01) {
          std::size_t length = manu.data.size() - bytePos - 1;
          std::size_t maxLength = (length < manu.data.size() - bytePos - 1 ? length : manu.data.size() - bytePos - 1);
          appleSegments.emplace_back(
            typeValue,
            subDataBigEndian(manu.data, bytePos + 1, maxLength)
          );
          bytePos += maxLength + 1;
        }
        // Parse according to Type-Length-Data
        else {
          std::uint8_t length = 0;
          bool lok = manu.data.uint8(bytePos + 1, length);
          if (lok) {
            std::size_t maxLength = (length < manu.data.size() - bytePos - 2 ? length : manu.data.size() - bytePos - 2);
            appleSegments.emplace_back(
              typeValue, subDataBigEndian(manu.data, bytePos + 2, maxLength)
            );
            bytePos += (maxLength + 2);
          }
        }
      }
    }
  }
  return appleSegments;
}


// Low level utility functions
// Exposed in API to allow others to use them

Data
subDataBigEndian(const Data& data, std::size_t offset, std::size_t length) noexcept
{
  return data.subdata(offset, length);
}

// TODO consider moving the below logic in to something like Data::subdataReversed()
Data 
subDataLittleEndian(const Data& data, std::size_t offset, std::size_t length) noexcept
{
  Data d;
  if (offset < 0 || length <= 0) {
    return d;
  }
  // If offset passed as -1 it will be MAX_LONG_LONG by itself, so test on its own first
  if (offset > data.size() || length + offset > data.size()) {
    return d;
  }
  // std::size_t position = offset + length - 1;
  d.appendReversed(data, offset, length);
  // for (std::size_t c = 0;c < length;c++) {
  //   d.append(data.at(position--));
  // }
  return d;
}

}
}
}
}