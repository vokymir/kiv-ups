#pragma once

#include "prsi/game/Model.hpp"
#include "prsi/interfaces/ILoggable.hpp"
#include "prsi/server/Session.hpp"
#include "prsi/util/Config.hpp"
#include <cstdint>
#include <map>
#include <memory>

using prsi::game::Model;
using prsi::util::Config;

namespace prsi::server {

class Server : public interfaces::ILoggable {
private:
  int server_fd_ = -1;
  int epoll_fd_ = -1;

  std::unique_ptr<Config> config_;
  std::map<int, std::unique_ptr<Session>> sessions_;
  std::unique_ptr<Model> model_;

public:
  Server();
  ~Server();

  void process_events();

private:
  int setup_server_socket();
  void accept_new_connection();
  bool handle_client_event(const int fd, uint32_t events);
  void remove_client(const int fd);
};

} // namespace prsi::server
