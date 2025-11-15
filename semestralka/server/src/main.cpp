
#include "server/Server.hpp"
#include "util/Config.hpp"
int main() {
  auto &cfg = prsi::util::Config::instance("ano");
  auto server = prsi::server::Server();
  server.setup();
}
