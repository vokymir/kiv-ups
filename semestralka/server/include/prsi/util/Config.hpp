#pragma once

#include <string>

namespace prsi::util {

// singleton
class Config {
private:
  Config();
  ~Config();

public:
  static Config &instance() {
    static Config cfg;
    return cfg;
  }
  // helper loader
  void loadFromFile(const std::string &path);

  // remove copy
  // Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // values
  std::string server_address = "127.0.0.1";
  int server_port = 10000;

  int poll_timeout_ms = 1500;
  int ping_interval_ms = 3000;

  int max_rooms = 10;
  int max_players = 50;

  std::string log_filename = "log.txt";
};

} // namespace prsi::util
