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
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // values
  std::string listenAddress = "127.0.0.1";
  int listenPort = 10000;

  int pollTimeoutMs = 1500;
  int pingIntervalMs = 3000;

  int maxRooms = 10;
  int maxPlayers = 50;

  std::string logFilename = "log.txt";
};

} // namespace prsi::util
