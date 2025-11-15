#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace prsi::util {

class Config {
private:
  // TODO: all needed variables here
  uint16_t port_ = 10512;
  int epoll_max_events_ = 32;
  int epoll_timeout_ms_ = 500;
  int max_clients_ = 10;
  int ping_timeout_ms_ = 5'000;
  int disconnect_timeout_ms_ = 30'000;

public:
  // TODO: all getters
  uint16_t port() const;
  int epoll_max_events() const;
  int epoll_timeout_ms() const;
  size_t max_clients() const;
  int ping_timeout_s() const;
  int disconnect_timeout_s() const;

private: // ONLY private constructor
  explicit Config(const std::string &filename);
  inline static bool initialized_ = false;

public: // make it SINGLETON
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  static const Config &instance(const std::string &filename = "") {
    if (!initialized_) {
      if (filename.empty()) {
        throw std::runtime_error(
            "First call of instance() MUST provide filename "
            "to get configuration from.");
      }
      initialized_ = true;
    }
    static Config cfg(filename);
    return cfg;
  }
};

} // namespace prsi::util
