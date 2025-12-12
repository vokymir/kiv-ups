#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>

namespace prsi {

class Config {
public:
  // IP
  std::string ip_ = "127.0.0.1";
  // PORT
  int port_ = 65'500;
  // EME
  // should be at least max_clients_ + 1 (listen)
  int epoll_max_events_ = 32;
  // ET
  int epoll_timeout_ms_ = 500;
  // MC
  int max_clients_ = 10;
  // PT
  int ping_timeout_ms_ = 2'000;
  // ST
  int sleep_timeout_ms_ = 10'000;
  // DT
  int death_timeout_ms_ = 30'000;
  // MR
  int max_rooms_ = 10;

public:
  // load config from file
  Config(const std::string &filename);
  // default config
  Config();
  ~Config() = default;

private:
  using Setter = void (Config::*)(const std::string &);
  static const std::unordered_map<std::string, Setter> setters_;

  // Individual setters
  void ip(const std::string &val) { ip_ = val; }
  void port(const std::string &val) { port_ = std::stoi(val); }
  void eme(const std::string &val) { epoll_max_events_ = std::stoi(val); }
  void et(const std::string &val) { epoll_timeout_ms_ = std::stoi(val); }
  void mc(const std::string &val) { max_clients_ = std::stoi(val); }
  void pt(const std::string &val) { ping_timeout_ms_ = std::stoi(val); }
  void st(const std::string &val) { sleep_timeout_ms_ = std::stoi(val); }
  void dt(const std::string &val) { death_timeout_ms_ = std::stoi(val); }
  void mr(const std::string &val) { max_rooms_ = std::stoi(val); }

  static std::string to_upper(const std::string &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
  }
};

} // namespace prsi
