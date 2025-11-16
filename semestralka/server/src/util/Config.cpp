#include "util/Config.hpp"
#include <cstdint>
#include <stdexcept>

namespace prsi::util {

uint16_t Config::port() const { return port_; }

int Config::epoll_max_events() const { return epoll_max_events_; }
int Config::epoll_timeout_ms() const { return epoll_timeout_ms_; }

int Config::max_clients() const { return max_clients_; }
int Config::ping_timeout_s() const { return ping_timeout_ms_ / 1000; }
int Config::disconnect_timeout_s() const {
  return disconnect_timeout_ms_ / 1000;
}

const Config &Config::instance(const std::string &filename) {
  if (!initialized_) {
    if (filename.empty()) {
      throw std::runtime_error("First call of instance() MUST have filename.");
    }
    initialized_ = true;
  }
  static Config cfg(filename);
  return cfg;
}

} // namespace prsi::util
