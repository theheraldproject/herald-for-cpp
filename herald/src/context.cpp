//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/context.h"
#include "herald/data/sensor_logger.h"

#include <iostream>

namespace herald {

// TODO filter this default logger by CONFIG_HERALD_LOG_LEVEL

class StdOutLoggingSink : public SensorLoggingSink {
public:
  StdOutLoggingSink(const std::string& subsystemFor, const std::string& categoryFor)
    : m_subsystem(subsystemFor), m_category(categoryFor)
  {
    ;
  }
  ~StdOutLoggingSink()
  {
    ;
  }

  void log(SensorLoggerLevel level, std::string message) override
  {
    std::string lvl = "info";
    switch (level) {
      case SensorLoggerLevel::debug:
        lvl = "debug";
        break;
      case SensorLoggerLevel::fault:
        lvl = "fault";
        break;
      default:
        break;
    }
    std::cout << m_subsystem << "," << m_category << ","
              << lvl << "," << message << std::endl;
  }

private:
  const std::string m_subsystem;
  const std::string m_category;
};

std::shared_ptr<SensorLoggingSink>
DefaultContext::getLoggingSink(const std::string& subsystemFor, const std::string& categoryFor)
{
  return std::make_shared<StdOutLoggingSink>(subsystemFor,categoryFor);
}

std::shared_ptr<BluetoothStateManager>
DefaultContext::getBluetoothStateManager()
{
  return nullptr;
}


}