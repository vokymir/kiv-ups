#pragma once

#include "game/Lobby.hpp"
#include "server/Client.hpp"
#include "util/Config.hpp"
#include <map>
#include <string>

namespace prsi::server {

class Server {
private:
  int epoll_fd_;
  int listen_fd_;
  std::map<int, Client> clients_;
  prsi::game::Lobby lobby_;

public:
  void setup(const util::Config &cfg);
  void accept_connection();
  void handle_client_data(int fd);
  void handle_clent_disconnect(int fd);
  void broadcast_to_room(prsi::game::Room *room, const std::string &msg,
                         int except_fd);

  void run();
};

} // namespace prsi::server
