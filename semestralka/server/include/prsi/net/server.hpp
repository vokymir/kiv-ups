#pragma once

#include "prsi/mgr/game_manager.hpp"
#include "prsi/net/session.hpp"
#include "prsi/util/config.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace prsi::net {

// Handle epoll, own sessions and game manager.
class Server {
private:
  // epoll
  int epoll_fd_ = -1;
  std::vector<epoll_event> events_;

  // sockets
  int listen_fd_ = -1;

  // orchestration
  bool running_ = false;

  // owns
  std::unique_ptr<mgr::Game_Manager> game_manager_;
  std::map<int, std::shared_ptr<Session>> sessions_;

public:
  Server(const util::Config &config);
  ~Server() = default;
  void run();

private:
  void setup();
  // return -1 on failure
  int set_fd_nonblocking(int fd);
  // return -1 on failure
  int set_epoll_events(int fd, uint32_t events, bool creating_new = false);

  // accept new connection
  void accept_connection();

  // find session by the file descriptor
  std::weak_ptr<Session> find_session(int fd);

  // session read message into its buffer
  void handle_session_read(std::weak_ptr<Session> session);
  // session process all complete messages inside its buffer
  void handle_session_process(std::weak_ptr<Session> session);
  // session send all messages inside its buffer
  void handle_session_send(std::weak_ptr<Session> session);

  // disconnect session from server
  void handle_disconnect(int fd);

private:
  // configuration
  int port_;
  int epoll_max_events_;
  int epoll_timeout_ms_;
  int max_clients_;
  int ping_timeout_ms_;
  int sleep_timeout_ms_;
  int death_timeout_ms_;
};

} // namespace prsi::net
