
#include "config.hpp"
#include "logger.hpp"
#include "server.hpp"

int main(int argc, char **argv) {
  prsi::Logger::info("Server started.");

  prsi::Config cfg("cfg");
  prsi::Server s{cfg};

  s.run();

  return 0;
}
