#pragma once

#include "config.hpp"
#include "room.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

namespace prsi {

// Handle epoll, own sessions and game manager.
class Server {
  friend class Player; // forward declare & befriend
  friend class Protocol;

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
  // categorize message, do what is appropriate for it
  void process_message(const std::vector<std::string> &msg,
                       std::shared_ptr<Player> p);
  // try flushing message to the socket
  void server_send(int fd);
  void disconnect(int fd);

  void maybe_ping(std::shared_ptr<Player> p);
  void check_pong(std::shared_ptr<Player> p);

  // friendly functions
  // enable EPOLLOUT for a socket
  void enable_sending(int fd);
  // disable EPOLLOUT for a socket
  void disable_sending(int fd);

  // net + game
private:
  // remove player from all rooms or lobby, notify others & disconnect player
  // from server
  void terminate_player(std::shared_ptr<Player> p);
  // remove player from room/lobby + notify others
  // WARN: it will erase the owning shared_ptr
  void remove_from_game_server(std::shared_ptr<Player> p);
  // disconnect client behind FD from server
  void close_connection(int fd);
  // broadcast to room with the exception of players with given fds
  void broadcast_to_room(std::shared_ptr<Room> r, const std::string &msg,
                         const std::vector<int> &except_fds);
  // do everything what is needed on leaving room - send all messages, notify
  // roommates. if player is not in room, throw
  void leave_room(std::shared_ptr<Player> p, std::shared_ptr<Room> r);

  // game
private:
  // list all players on the server
  std::vector<std::shared_ptr<Player>> list_players();
  // count all players everywhere
  int count_players() const;
  // count all rooms
  int count_rooms() const;

  // find player anywhere on server & return weak ptr to them
  std::weak_ptr<Player> find_player(int fd);
  std::weak_ptr<Player> find_player(const std::string &nick);
  // at which state the player is
  Player_Location where_player(std::shared_ptr<Player> p);

  // handlers
private:
  // handler for any incoming message
  using Handler = void (Server::*)(const std::vector<std::string> &,
                                   std::shared_ptr<Player>);
  // lookup table
  // store all handlers for incoming messages
  // all handlers have the capability to terminate player, if invoked
  // incorrectly = bad time / bad syntax
  static const std::unordered_map<std::string, Handler> handlers_;

  // set last pong
  void handle_pong(const std::vector<std::string> &msg,
                   std::shared_ptr<Player> p);
  void handle_name(const std::vector<std::string> &msg,
                   std::shared_ptr<Player> p);
  void handle_list_rooms(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p);
  void handle_join_room(const std::vector<std::string> &msg,
                        std::shared_ptr<Player> p);
  void handle_create_room(const std::vector<std::string> &msg,
                          std::shared_ptr<Player> p);
  void handle_leave_room(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p);
  void handle_room_info(const std::vector<std::string> &msg,
                        std::shared_ptr<Player> p);
  void handle_state(const std::vector<std::string> &msg,
                    std::shared_ptr<Player> p);
  void handle_play(const std::vector<std::string> &msg,
                   std::shared_ptr<Player> p);

  // player manipulation
private:
  template <typename Pred>
  void move_player(Pred pred, std::vector<std::shared_ptr<Player>> &from,
                   std::vector<std::shared_ptr<Player>> &to) {
    auto it = std::find_if(from.begin(), from.end(), pred);
    if (it == from.end()) {
      throw std::runtime_error("Cannot move player - wasn't found.");
    }
    to.push_back(std::move(*it));
    from.erase(it);
  }

  // convenience functions for moving player
  void move_player_by_fd(int fd, std::vector<std::shared_ptr<Player>> &from,
                         std::vector<std::shared_ptr<Player>> &to);
  void move_player_by_nick(const std::string &nick,
                           std::vector<std::shared_ptr<Player>> &from,
                           std::vector<std::shared_ptr<Player>> &to);

  // erase from any vector
  void erase_by_fd(std::vector<std::shared_ptr<Player>> &v, int fd) {
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](auto &sp) { return sp->fd() == fd; }),
            v.end());
  }

private:
  // configuration
  std::string ip_;
  int port_;
  int epoll_max_events_; // must be at least max_clients_ + 1 (listen socket)
  int epoll_timeout_ms_;
  int max_clients_;
  int max_rooms_;
  int ping_timeout_ms_;
  int sleep_timeout_ms_;
  int death_timeout_ms_;
  int players_in_game_ = 2;
};

} // namespace prsi
