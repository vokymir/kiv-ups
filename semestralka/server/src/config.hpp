#pragma once

#include <string>

namespace prsi {

class Config {
public:
  std::string ip_ = "127.0.0.1";
  int port_ = 65'500;
  int epoll_max_events_ = 32; // should be at least max_clients_ + 1 (listen)
  int epoll_timeout_ms_ = 500;
  int max_clients_ = 10;
  int ping_timeout_ms_ = 2'000;
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

} // namespace prsi
