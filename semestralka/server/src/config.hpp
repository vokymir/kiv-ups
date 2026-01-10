#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>

namespace prsi {

class Config {
public:
  // IP
  // the ip address on which the server is listening, default is all
  std::string ip_ = "0.0.0.0";
  // PORT
  // the port on which the server is listening, the default value is 3750,
  // because if you put this value inside classic seven-segment display
  // calculator and look at it upside down, you get the word OSLE. (translate at
  // your own risk)
  int port_ = 3'750;
  // EME
  // maximum number of events to which the epoll would listen
  // should be at least 2 * max_clients_ + 1 (listen)
  // 2 times because each client could have reconnect timer
  int epoll_max_events_ = 32;
  // ET
  // in how many miliseconds would epoll_wait() stop be blocking
  int epoll_timeout_ms_ = 500;
  // MC
  // how many clients could be connected to the server at once
  // at least 2 are required (one game-room consists of two players)
  int max_clients_ = 10;
  // PT
  // how often send pings
  int ping_timeout_ms_ = 2'000;
  // ST
  // in how many milliseconds without PONG is client considered asleep
  int sleep_timeout_ms_ = 5'000;
  // DT
  // in how many milliseconds without PONG is client considered dead
  // NOTE: death is stronger than kick, after death timeout is over, the player
  // is definitely removed from the server
  int death_timeout_ms_ = 180'000;
  // MR
  // max number of rooms, at least 1 is needed to play a game
  int max_rooms_ = 10;
  // KT
  // kic-out timer, in how many ms would you automatic timer kick out of server.
  // currently not used really
  int kick_timer_ms_ = 180'000;

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
  void kt(const std::string &val) { kick_timer_ms_ = std::stoi(val); }

  static std::string to_upper(const std::string &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
  }
};

} // namespace prsi
