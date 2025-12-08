#pragma once

#include <cstdint>
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
  int brief_disconnect_timeout_ms_ = 10'000;
  int disconnect_timeout_ms_ = 30'000;
  int max_rooms_ = 10;

public:
  // TODO: all getters
  uint16_t port() const;
  int epoll_max_events() const;
  int epoll_timeout_ms() const;
  int max_clients() const;
  int ping_timeout_s() const;
  int brief_disconnect_timeout_s() const;
  int disconnect_timeout_s() const;
  int max_rooms() const;

private: // ONLY private constructor
  explicit Config(const std::string &filename);
  inline static bool initialized_ = false;

public: // make it SINGLETON
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;
  // can only get one instance of config, defined by filename in the first call
  static const Config &instance(const std::string &filename = "");
};

} // namespace prsi::util
