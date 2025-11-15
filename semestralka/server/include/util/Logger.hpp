#pragma once

#include <string>

namespace prsi::util {

class Logger {
public:
  static void info(std::string msg);
  static void warn(std::string msg);
  static void error(std::string msg);
};

} // namespace prsi::util
