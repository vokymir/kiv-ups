#pragma once

#include "config.hpp"
#include "player.hpp"
#include "room.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
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
  // initialize member variables
  Server(const Config &config);
  ~Server();
  // the main server loop - wait on epoll socket events & handle them
  void run();

  // net
private:
  // setup the server on construction
  void setup();
  // must set all fd as non-blocking to work well pseudo-parallel
  // return -1 on failure
  int set_fd_nonblocking(int fd);
  // return -1 on failure
  int set_epoll_events(int fd, uint32_t events, bool creating_new_ev = false);

  // accept new connection
  void accept_connection();
  void receive(int fd);
  void send(int fd); // TODO: placeholder
  void disconnect(int fd);

  void maybe_ping(std::shared_ptr<Player> p);
  void check_pong(std::shared_ptr<Player> p);

  // net + game
private:
  // remove player from all rooms or lobby, notify others & disconnect player
  // from server
  void terminate_player(std::shared_ptr<Player> p);
  // remove player from room/lobby + notify others
  // WARN: it will erase the owning shared_ptr
  void remove_from_game(std::shared_ptr<Player> p); // TODO: not done

  // game
private:
  // list all players on the server
  std::vector<std::shared_ptr<Player>> list_players();
  // count all players everywhere
  int count_players() const;
  // count players in all states:
  // unnamed: 2, lobby: 1, ...
  // NOTE: is it even useful? its really unefficient
  std::map<Player_State, int> count_players_by_state(); // TODO: not done
  // count all rooms
  int count_rooms() const;

  // find player anywhere on server & return weak ptr to them
  std::weak_ptr<Player> find_player(int fd);
  // at which state the player is
  Player_Location where_player(std::shared_ptr<Player> p);

private:
  // configuration
  std::string ip_;
  int port_;
  int epoll_max_events_; // must be at least max_clients_ + 1 (listen socket)
  int epoll_timeout_ms_;
  int max_clients_;
  int ping_timeout_ms_;
  int sleep_timeout_ms_;
  int death_timeout_ms_;
};

} // namespace prsi
