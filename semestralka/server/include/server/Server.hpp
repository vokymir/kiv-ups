#pragma once

#include "game/Lobby.hpp"
#include "server/Client.hpp"
#include "util/Config.hpp"
#include <map>
#include <string>
#include <sys/epoll.h>
#include <vector>

namespace prsi::server {

class Server {
private:
  int epoll_fd_ = -1;
  int listen_fd_ = -1;
  std::vector<epoll_event> events_;

  const util::Config &cfg_ = util::Config::instance();
  bool running_ = false;

  std::map<int, Client> clients_;
  prsi::game::Lobby lobby_;

public:
  Server();
  ~Server() = default;
  void setup();
  void accept_connection();
  void handle_client_data(int fd);
  void handle_clent_disconnect(int fd);
  void check_timeouts();
  void broadcast_to_room(prsi::game::Room *room, const std::string &msg,
                         int except_fd);

  void run();
};

} // namespace prsi::server
