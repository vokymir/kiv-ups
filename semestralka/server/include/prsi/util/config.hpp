#pragma once

#include <cstdint>
#include <string>

namespace prsi::util {

class Config {
public:
  uint16_t port_ = 65'500;
  int epoll_max_events_ = 32;
  int epoll_timeout_ms_ = 500;
  int max_clients_ = 10;
  int ping_timeout_ms_ = 5'000;
  int sleep_timeout_ms_ = 10'000;
  int death_timeout_ms_ = 30'000;
  int max_rooms_ = 10;

public:
  // load config from file
  Config(std::string &filename);
  // default config
  Config();
  ~Config() = default;
};

} // namespace prsi::util
