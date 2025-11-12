#pragma once

#include "prsi/util/Config.hpp"
#include <string>

namespace prsi::util {

// singleton
class Logger {
private:
  Logger();
  ~Logger();

public:
  static Logger &instance() {
    static Logger lgr;
    return lgr;
  }
  // remove copy
  // Logger(const Logger &) = delete;
  // Logger &operator=(const Logger &) = delete;

  // useful functions
  void info(std::string log);
  void warn(std::string log);
  void error(std::string log);

private:
  std::string filename_ = Config::instance().log_filename;
};

} // namespace prsi::util
