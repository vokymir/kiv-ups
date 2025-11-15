#pragma once

#include <stdexcept>
#include <string>

namespace prsi::util {

class Config {
private:
  // TODO: all needed variables here
  int port_;

public:
  // TODO: all getters
  int port() const;

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
