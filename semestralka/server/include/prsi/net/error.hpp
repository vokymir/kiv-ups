#pragma once

#include <stdexcept>
#include <string>

namespace prsi::net {

class Net_Error : public std::runtime_error {
public:
  explicit Net_Error(const char *message) : std::runtime_error(message) {}
  explicit Net_Error(std::string message)
      : std::runtime_error(std::move(message)) {}
};

} // namespace prsi::net
