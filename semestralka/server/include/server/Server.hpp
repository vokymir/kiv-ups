#pragma once

#include "game/Lobby.hpp"
#include "server/Client.hpp"
#include "server/Message.hpp"
#include "util/Config.hpp"
#include <cstdint>
#include <map>
#include <sys/epoll.h>
#include <vector>

namespace prsi::server {

class Server {
private:
  const util::Config &cfg_ = util::Config::instance();
  // epoll
  int epoll_fd_ = -1;
  std::vector<epoll_event> events_;
  // sockets
  int listen_fd_ = -1;
  std::map<int, Client> clients_;
  // game
  bool running_ = false;
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
  void check_timeouts();

public:
  void handle_client_disconnect(int fd);
  void send_message(int fd, const Server_Message &msg);
  void broadcast_to_room(const prsi::game::Room *room,
                         const Server_Message &msg, int except_fd);
  const game::Lobby &lobby() const;
};

} // namespace prsi::server
