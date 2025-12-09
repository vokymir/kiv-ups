#include "prsi/net/server.hpp"
#include "prsi/util/config.hpp"

namespace prsi::net {

Server::Server(const util::Config &cfg)
    : port_(cfg.port_), epoll_max_events_(cfg.epoll_max_events_),
      epoll_timeout_ms_(cfg.epoll_timeout_ms_), max_clients_(cfg.max_clients_),
      ping_timeout_ms_(cfg.ping_timeout_ms_),
      sleep_timeout_ms_(cfg.sleep_timeout_ms_),
      death_timeout_ms_(cfg.death_timeout_ms_) {
  events_.reserve(epoll_max_events_);
  setup();
}

} // namespace prsi::net
