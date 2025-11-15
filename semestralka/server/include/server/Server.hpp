#pragma once

#include "game/Lobby.hpp"
#include "server/Client.hpp"
#include "server/Message.hpp"
#include "util/Config.hpp"
#include <cstdint>
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
  void run();

private:
  void setup();
  bool set_fd_nonblocking(int fd);
  bool set_epoll_events(int fd, uint32_t events, bool creating_new = false);
  void accept_connection();
  void handle_client_read(int fd);
  void handle_client_write(int fd);
  void handle_client_disconnect(int fd);
  void check_timeouts();
  void send_message(int fd, Server_Message msg);
  void broadcast_to_room(prsi::game::Room *room, const std::string &msg,
                         int except_fd);
};

} // namespace prsi::server
