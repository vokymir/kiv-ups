#include "server.hpp"
#include <csignal>

int main(int argc, char **argv) {
  std::signal(SIGINT, Server::handle_sigint);

  auto server = Server{10000, 15000, true};
  server.run();

  return 0;
}
