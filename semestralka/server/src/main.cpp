
#include "server/Server.hpp"
#include "util/Config.hpp"
int main() {
  prsi::util::Config::instance("tady je to nutne, aby se setupnul config");
  auto server = prsi::server::Server();
}
