//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_BYTE_ARRAY_PRINTER_H
#define HERALD_BYTE_ARRAY_PRINTER_H

namespace herald {
namespace util {

template <typename CtxT>
class ByteArrayPrinter {
public:
  ByteArrayPrinter(CtxT& context)
    : ctx(context)
      HLOGGERINIT(ctx,"util","ByteArrayPrinter")
  {
    ;
  }

  void print(const std::array<unsigned char,16>& data, const std::size_t initialByteID) noexcept {
    static constexpr char hexChars[] {
      '0','1','2','3','4','5','6','7',
      '8','9','a','b','c','d','e','f'
    };
    std::string ci = std::to_string(initialByteID);
    ci += ": ";
    ci += hexChars[0x0F & (std::size_t(data[ 0]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 0])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 1]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 1])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 2]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 2])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 3]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 3])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 4]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 4])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 5]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 5])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 6]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 6])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 7]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 7])];
    ci += "  ";
    ci += hexChars[0x0F & (std::size_t(data[ 8]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 8])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[ 9]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[ 9])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[10]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[10])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[11]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[11])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[12]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[12])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[13]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[13])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[14]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[14])];
    ci += " ";
    ci += hexChars[0x0F & (std::size_t(data[15]) >> 4)];
    ci += hexChars[0x0F & std::size_t(data[15])];
    HTDBG(ci);
  }
private:
  CtxT& ctx;
  HLOGGER(CtxT);
};

}
}

#endif