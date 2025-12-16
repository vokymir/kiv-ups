
#include "config.hpp"
#include "logger.hpp"
#include "server.hpp"

int main(int argc, char **argv) {
  prsi::Logger::info("Server started.");

  // load config
  prsi::Config cfg{};
  if (argc > 1) {
    prsi::Config cfg(argv[1]);
  }

  prsi::Server s{cfg};

  s.run();

  return 0;
}
