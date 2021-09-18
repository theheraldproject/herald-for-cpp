//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SENSOR_LOGGER_H
#define HERALD_SENSOR_LOGGER_H

#include "../datatype/bluetooth_state.h"

#include <string>
#include <memory>
#include <ostream>
#include <sstream>

// Zephyr compile workaround. Not ideal.
// #ifndef HERALD_LOG_LEVEL
// #define HERALD_LOG_LEVEL 4
// #endif

#ifdef HERALD_LOG_LEVEL

// Defines for within Impl class definitions
#if HERALD_LOG_LEVEL != 0
#define HLOGGER(_ctxT) \
  herald::data::SensorLogger<typename _ctxT::logging_sink_type> logger;
#define HLOGGERINIT(_ctx,_subsystem,_category) ,logger(_ctx.getLoggingSink(),_subsystem,_category)
#define HLOGGERINLINE(_ctx,_subsystem,_category) \
  herald::data::SensorLogger logger(_ctx.getLoggingSink(),_subsystem,_category);
#endif

// HDBG Defines for within main class (more common)
// HTDBG Defines for within Impl class
#if HERALD_LOG_LEVEL == 4
#define HDBG(_msg, ...) mImpl->logger.debug(_msg, ##__VA_ARGS__);
#define HTDBG(_msg, ...) logger.debug(_msg, ##__VA_ARGS__);
#define HLOG(_msg, ...) mImpl->logger.info(_msg, ##__VA_ARGS__);
#define HTLOG(_msg, ...) logger.info(_msg, ##__VA_ARGS__);
#define HERR(_msg, ...) mImpl->logger.fault(_msg, ##__VA_ARGS__);
#define HTERR(_msg, ...) logger.fault(_msg, ##__VA_ARGS__);
#endif

#if HERALD_LOG_LEVEL == 3
#define HDBG(...) /* No debug log */
#define HTDBG(...) /* No debug log */
#define HLOG(_msg, ...) mImpl->logger.info(_msg, ##__VA_ARGS__);
#define HTLOG(_msg, ...) logger.info(_msg, ##__VA_ARGS__);
#define HERR(_msg, ...) mImpl->logger.fault(_msg, ##__VA_ARGS__);
#define HTERR(_msg, ...) logger.fault(_msg, ##__VA_ARGS__);
#endif

// This 'WARN' exists for runtime valid logging. E.g. contacts.log to RTT on Zephyr
#if HERALD_LOG_LEVEL == 2
#define HDBG(...) /* No debug log */
#define HTDBG(...) /* No debug log */
#define HLOG(_msg, ...) mImpl->logger.info(_msg, ##__VA_ARGS__);
#define HTLOG(_msg, ...) logger.info(_msg, ##__VA_ARGS__);
#define HERR(_msg, ...) mImpl->logger.fault(_msg, ##__VA_ARGS__);
#define HTERR(_msg, ...) logger.fault(_msg, ##__VA_ARGS__);
#endif

#if HERALD_LOG_LEVEL == 1
#define HDBG(...) /* No debug log */
#define HTDBG(...) /* No debug log */
#define HLOG(...) /* No info log */
#define HTLOG(...) /* No info log */
#define HERR(_msg, ...) mImpl->logger.fault(_msg, ##__VA_ARGS__);
#define HTERR(_msg, ...) logger.fault(_msg, ##__VA_ARGS__);
#endif

#if HERALD_LOG_LEVEL == 0

#define HLOGGER(_ctxT) /* No logger instance */
#define HLOGGERINIT(...) /* No logger init */
#define HDBG(...) /* No debug log */
#define HERR(...) /* No error log */
#define HLOG(...) /* No info log */
#define HTDBG(...) /* No debug log */
#define HTERR(...) /* No error log */
#define HTLOG(...) /* No info log */

#endif

#else

#define HLOGGER(_ctxT) /* No logger instance */
#define HLOGGERINIT(...) /* No logger init */
#define HDBG(...) /* No debug log */
#define HERR(...) /* No error log */
#define HLOG(...) /* No info log */
#define HTDBG(...) /* No debug log */
#define HTERR(...) /* No error log */
#define HTLOG(...) /* No info log */

#endif

namespace herald {

namespace data {

enum class SensorLoggerLevel : int {
  debug, info, fault
};

/*
class LoggingSink {
public:
  LoggingSink() = default;
  ~LoggingSink() = default;

  void log(const std::string& subsystem, const std::string& category, SensorLoggerLevel level, std::string message);
};
*/

// NOTE: HEADER ONLY CLASS AS IT USES VARIABLE TEMPLATE ARGS FOR LOGGING

// class SensorLoggingSink {
// public:
//   SensorLoggingSink() = default;
//   virtual ~SensorLoggingSink() = default;

//   virtual void log(SensorLoggerLevel level, std::string message) = 0;
// };

namespace {
  
  [[maybe_unused]]
  void tprintf(std::stringstream& os, const std::string& format) // base function
  {
    std::size_t pos = 0;
    for ( auto c : format ) {
      if ( c == '{' ) {
        if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
          tprintf(os, format.substr(pos + 2)); // recursive call
        } else {
          tprintf(os, format.substr(pos + 1)); // recursive call
        }
        return;
      }
      os << c;
      ++pos;
    }
  }

  /// MARK: Individual value streaming support

  /// \brief Fallback method that assumes a << operator exists for type T.
  template <typename T>
  void tprintValue(std::stringstream& os, T value)
  {
    os << value;
  }
  
  // template <typename T>
  // auto tprintValue(std::stringstream& os, T value) -> decltype(operator<<(os,value), void())
  // {
  //   os << value;
  // }

  // /// \brief Partial specialisation for types that std::to_string supports
  // template <typename T>
  // auto tprintValue(std::stringstream& os, T value) -> decltype(std::to_string(value), void())
  // {
  //   os << std::to_string(value);
  // }
  
  [[maybe_unused]]
  void tprintValue(std::stringstream& os, std::uint8_t value)
  {
    // only uint16 and above on zephyr has a stream operator
    os << std::uint16_t(value);
  }
  
  [[maybe_unused]]
  void tprintValue(std::stringstream& os, std::int8_t value)
  {
    // only int16 and above on zephyr has a stream operator
    os << std::int16_t(value);
  }
  
  /// \brief Prints an int value to the stream
  /// \note Also covers int32_t and int64_t
  [[maybe_unused]]
  void tprintValue(std::stringstream& os, int value)
  {
    os << std::int64_t(value);
  }
  
  [[maybe_unused]]
  void tprintValue(std::stringstream& os, double value)
  {
    // double may not be supported depending on Zephyr compile flags
    // TODO check for support for printf(double) rather than just assume it is not there
    os << "~" << ((int)value) << "d";
  }
 
  // template<typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, std::uint8_t value, Targs... Fargs) // recursive variadic function
  // {
  //   std::size_t pos = 0;
  //   for ( auto c : format ) {
  //     if ( c == '{' ) {
  //       os << std::uint16_t(value);
  //       if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
  //         tprintf(os, format.substr(pos + 2), Fargs...); // recursive call
  //       } else {
  //         tprintf(os, format.substr(pos + 1), Fargs...); // recursive call
  //       }
  //       return;
  //     }
  //     os << c;
  //     ++pos;
  //   }
  // }
 
  // template<typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, std::int8_t value, Targs... Fargs) // recursive variadic function
  // {
  //   std::size_t pos = 0;
  //   for ( auto c : format ) {
  //     if ( c == '{' ) {
  //       os << std::int16_t(value);
  //       if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
  //         tprintf(os, format.substr(pos + 2), Fargs...); // recursive call
  //       } else {
  //         tprintf(os, format.substr(pos + 1), Fargs...); // recursive call
  //       }
  //       return;
  //     }
  //     os << c;
  //     ++pos;
  //   }
  // }
 
  // template<typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, double value, Targs... Fargs) // recursive variadic function
  // {
  //   std::size_t pos = 0;
  //   for ( auto c : format ) {
  //     if ( c == '{' ) {
  //       os << "~" << (int)(value) << "d";
  //       if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
  //         tprintf(os, format.substr(pos + 2), Fargs...); // recursive call
  //       } else {
  //         tprintf(os, format.substr(pos + 1), Fargs...); // recursive call
  //       }
  //       return;
  //     }
  //     os << c;
  //     ++pos;
  //   }
  // }
 
  // template<typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, const std::string& value, Targs... Fargs) // recursive variadic function
  // {
  //   std::size_t pos = 0;
  //   for ( auto c : format ) {
  //     if ( c == '{' ) {
  //       os << value;
  //       if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
  //         tprintf(os, format.substr(pos + 2), Fargs...); // recursive call
  //       } else {
  //         tprintf(os, format.substr(pos + 1), Fargs...); // recursive call
  //       }
  //       return;
  //     }
  //     os << c;
  //     pos++;
  //   }
  // }
 
  // typename std::enable_if_t<std::is_convertible<T, std::string>::value, std::string>

  template<typename T>
  void tprintf(std::stringstream& os, const std::string& format, T value) // recursive variadic function
  {
    std::size_t pos = 0;
    for ( auto c : format ) {
      if ( c == '{' ) {
        tprintValue(os,value);
        if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
          tprintf(os, format.substr(pos + 2)); // recursive call
        } else {
          tprintf(os, format.substr(pos + 1)); // recursive call
        }
        return;
      }
      os << c;
      ++pos;
    }
  }

  template<typename FirstT, typename SecondT, typename... RestT>
  void tprintf(std::stringstream& os, const std::string& format, FirstT first, SecondT second, RestT... rest)
  {
    std::size_t pos = 0;
    for ( auto c : format ) {
      if ( c == '{' ) {
        tprintValue(os,first);
        if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
          tprintf(os, format.substr(pos + 2), second, rest...); // recursive call
        } else {
          tprintf(os, format.substr(pos + 1), second, rest...); // recursive call
        }
        return;
      }
      os << c;
      ++pos;
    }
  }

  // template<typename T, typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, T value, Targs... Fargs) // recursive variadic function
  // {
  //   std::size_t pos = 0;
  //   for ( auto c : format ) {
  //     if ( c == '{' ) {
  //       os << value;
  //       if (format.size() > pos + 1 && format.at(pos + 1) == '}') {
  //         tprintf(os, format.substr(pos + 2), Fargs...); // recursive call
  //       } else {
  //         tprintf(os, format.substr(pos + 1), Fargs...); // recursive call
  //       }
  //       return;
  //     }
  //     os << c;
  //     pos++;
  //   }
  // }

  // // G++ deduction guide workaround - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80438
  // template<typename T, typename... Targs>
  // void tprintf(std::stringstream& os, const std::string& format, Targs... Fargs)
  // {
  //   tprintf(os, format, Fargs...);
  // }
  
}

template <typename LoggingSinkT>
class SensorLogger {
public:
  SensorLogger(LoggingSinkT& sink, std::string subsystem, std::string category) noexcept
    : mSink(sink), mSubsystem(subsystem), mCategory(category)
  {
    ;
  }

  SensorLogger(const SensorLogger& other) noexcept
    : mSink(other.mSink), mSubsystem(other.mSubsystem), mCategory(other.mCategory)
  {
    ;
  }

  SensorLogger(SensorLogger&& other) noexcept
    : mSink(other.mSink), mSubsystem(other.mSubsystem), mCategory(other.mCategory)
  {
    ;
  }

  SensorLogger& operator=(const SensorLogger& other) noexcept
  {
    mSink = other.mSink;
    mSubsystem = other.mSubsystem;
    mCategory = other.mCategory;
    return *this;
  }

  SensorLogger& operator=(SensorLogger&& other) noexcept
  {
    mSink = other.mSink;
    mSubsystem = other.mSubsystem;
    mCategory = other.mCategory;
    return *this;
  }
  
  // TODO consider supporting multiple sinks in the context - E.g. USB UART and log file

  ~SensorLogger() noexcept = default;

  // use std::format to generate the string
  // std::format in C++20, fmt::format library before that
  // Note: C++11 Variadic template parameter pack expansion
  template <typename ... Types>
  void debug(const std::string& message, const Types&... args) const noexcept {
    constexpr int size = sizeof...(args);
    if constexpr (0 == size) {
      log(SensorLoggerLevel::debug,message);
    } else {
      std::stringstream os;
      tprintf(os,message,args...);
      os << std::ends;
      log(SensorLoggerLevel::debug, os.str());
    }
  }

  template <typename ... Types>
  void info(const std::string& message, const Types&... args) const noexcept {
    constexpr int size = sizeof...(args);
    if constexpr (0 == size) {
      log(SensorLoggerLevel::debug,message);
    } else {
      std::stringstream os;
      tprintf(os,message,args...);
      os << std::ends;
      log(SensorLoggerLevel::info, os.str());
    }
  }

  template <typename ... Types>
  void fault(const std::string& message, const Types&... args) const noexcept {
    constexpr int size = sizeof...(args);
    if constexpr (0 == size) {
      log(SensorLoggerLevel::debug,message);
    } else {
      std::stringstream os;
      tprintf(os,message,args...);
      os << std::ends;
      log(SensorLoggerLevel::fault, os.str());
    }
  }

private:
  inline void log(SensorLoggerLevel lvl, const std::string msg) const noexcept {
    mSink.log(mSubsystem, mCategory, lvl, msg);
  }

  LoggingSinkT& mSink;
  std::string mSubsystem;
  std::string mCategory;
};

} // end namespace
} // end namespace

#endif