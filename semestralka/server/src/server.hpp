#pragma once

#include "config.hpp"
#include "player.hpp"
#include "room.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace prsi {

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
  std::vector<std::shared_ptr<Player>> unnamed_;
  std::vector<std::shared_ptr<Player>> lobby_;
  std::vector<std::shared_ptr<Room>> rooms_;

public:
  Server(const Config &config);
  ~Server() = default;
  void run();

  // net
private:
  // setup the server on construction
  void setup();
  // return -1 on failure
  int set_fd_nonblocking(int fd);
  // return -1 on failure
  int set_epoll_events(int fd, uint32_t events, bool creating_new = false);

  // accept new connection
  void accept_connection();

  // game
private:
  // count all players everywhere
  int count_players() const;
  // count players in all states:
  // unnamed: 2, lobby: 1, ...
  std::map<Player_State, int> count_player_states();
  // count all rooms
  int count_rooms() const;

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

} // namespace prsi
