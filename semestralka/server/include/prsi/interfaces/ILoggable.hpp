#pragma once

#include "prsi/util/Logger.hpp"

using prsi::util::Logger;

namespace prsi::interfaces {

class ILoggable {
protected:
  Logger &logger_ = Logger::instance();
};

} // namespace prsi::interfaces
