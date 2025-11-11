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
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // useful functions
  void Info(std::string log);
  void Warn(std::string log);
  void Error(std::string log);

private:
  std::string filename_ = Config::instance().logFilename;
};

} // namespace prsi::util
