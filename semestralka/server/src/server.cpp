#include "server.hpp"
#include "card.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "player.hpp"
#include "protocol.hpp"
#include "room.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

namespace prsi {

// static part

const std::unordered_map<std::string, Server::Handler> Server::handlers_ = {
    {"PONG", &Server::handle_pong},
    {"NAME", &Server::handle_name},
    {"LIST_ROOMS", &Server::handle_list_rooms},
    {"JOIN_ROOM", &Server::handle_join_room},
    {"CREATE_ROOM", &Server::handle_create_room},
    {"LEAVE_ROOM", &Server::handle_leave_room},
    {"ROOM_INFO", &Server::handle_room_info},
    {"STATE", &Server::handle_state},
    {"PLAY", &Server::handle_play},
    {"DRAW", &Server::handle_draw},
};

// other

Server::~Server() {
  // close all connections
  for (auto &p : list_players()) {
    terminate_player(p);
  }
  // close listen socket
  close(listen_fd_);
  // close epoll socket
  close(epoll_fd_);
}

Server::Server(const Config &cfg)
    : port_(cfg.port_), epoll_max_events_(cfg.epoll_max_events_),
      epoll_timeout_ms_(cfg.epoll_timeout_ms_), max_clients_(cfg.max_clients_),
      ping_timeout_ms_(cfg.ping_timeout_ms_),
      sleep_timeout_ms_(cfg.sleep_timeout_ms_),
      death_timeout_ms_(cfg.death_timeout_ms_), ip_(cfg.ip_),
      max_rooms_(cfg.max_rooms_) {

  events_.resize(epoll_max_events_);
  setup();
}

void Server::run() {
  running_ = true;
  while (running_) {
    // wait for n events to happen
    // n it at most epoll_max_events_
    // if dont have enough events before timeout, stops either
    int n = epoll_wait(epoll_fd_, events_.data(), epoll_max_events_,
                       epoll_timeout_ms_);

    // some fail in epoll_wait
    if (n == -1) {
      if (errno == EINTR) {
        continue; // ignoring interrupts
      } // report anything else
      throw std::runtime_error("epoll_wait failed.");
    }

    // check all happened events
    for (int i = 0; i < n; i++) {
      epoll_event &ev = events_[i];

      if (ev.data.fd == listen_fd_) { // NEW CONNECTION
        accept_connection();

      } else if (ev.events & EPOLLIN) { // RECV
        receive(ev.data.fd);

      } else if (ev.events & EPOLLOUT) { // SEND
        server_send(ev.data.fd);

      } else if (ev.events & (EPOLLHUP | EPOLLERR)) { // DISCONNECT
        disconnect(ev.data.fd);
      }
    }

    // check for timeouts
    for (auto &p : list_players()) {
      maybe_ping(p);
      check_pong(p);
    }
  }
}

void Server::setup() {
  // create socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1) {
    throw std::runtime_error("Cannot create listen socket.");
  }

  // set socket options
  int opt = 1;
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    throw std::runtime_error("Cannot set socket options.");
  }

  // set non blocking
  if (set_fd_nonblocking(listen_fd_) == -1) {
    throw std::runtime_error("Cannot set listen socket non-blocking.");
  }

  // bind configuration
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  // set port
  addr.sin_port = htons(port_);
  // set IP address
  if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) <= 0) {
    throw std::runtime_error("Invalid IP address format or conversion error.");
  }

  if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) != 0) {
    throw std::runtime_error("Cannot bind listen socket.");
  }

  // set listen to only hardware limited number of connections
  if (listen(listen_fd_, SOMAXCONN) == -1) {
    throw std::runtime_error("Cannot listen.");
  }

  // NOTE: is used create1, because is newer & better
  epoll_fd_ = epoll_create1(0);

  if (epoll_fd_ == -1) {
    Logger::error("epoll_create failed. errno {}: {}", errno,
                  std::strerror(errno));
    throw std::runtime_error("Cannot create epoll.");
  }

  if (set_epoll_events(listen_fd_, EPOLLIN, true) == -1) {
    throw std::runtime_error("Cannot add listening socket to epoll.");
  }

  Logger::info("Server now listen on IP={}, PORT={}", ip_, port_);
}

int Server::set_fd_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }

  // reuse return value
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int Server::set_epoll_events(int fd, uint32_t events, bool creating) {
  epoll_event ev{};
  ev.events = events;
  ev.data.fd = fd;

  int opt = creating ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  // reuse return value
  return epoll_ctl(epoll_fd_, opt, fd, &ev);
}

void Server::accept_connection() {
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);

  int client_fd = accept(listen_fd_, (sockaddr *)&client_addr, &addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    Logger::error("accept() failed");
    return;
  }

  // do we have space for new connection?
  if (count_players() >= max_clients_) {
    close(client_fd);
    Logger::warn("Max clients reached, rejecting connection");
    return;
  }

  // setup client socket
  if (set_fd_nonblocking(client_fd))
    throw std::runtime_error("set fd nonblocking failed for fd=" +
                             std::to_string(client_fd));

  // add to epoll
  if (set_epoll_events(client_fd, EPOLLIN, true) == -1) {
    close(client_fd);
    Logger::error("Cannot add client to epoll, closing connection for fd={}",
                  client_fd);
    return;
  }

  // create new client
  auto player = std::make_shared<Player>(*this, client_fd);
  unnamed_.emplace_back(player);

  Logger::info("New client connected, fd={}", client_fd);
}

void Server::receive(int fd) {
  auto weak_p = find_player(fd);
  auto p = weak_p.lock();
  if (!p) {
    Logger::error("Receive: Player with id={} was not found anywhere.",
                  std::to_string(fd));
    close_connection(fd);
    return;
  }

  try { // player receive
    p->receive();

    // any exception = terminate
  } catch (const std::exception &ex) {
    Logger::error("Cannot receive from client fd={}, because: '{}'.", p->fd(),
                  ex.what());
    terminate_player(p);
  }

  try { // process messages

    auto msg = p->complete_recv_msg();
    while (!msg.empty()) {
      process_message(msg, p);

      msg = p->complete_recv_msg();
    }

    // received invalid message - doesn't start with magic
  } catch (const std::exception &ex) {
    Logger::error("Invalid message received from fd={}, what? {}", p->fd(),
                  ex.what());
    terminate_player(p);
  }
}

void Server::server_send(int fd) {
  auto weak_p = find_player(fd);
  auto p = weak_p.lock();
  if (!p) {
    Logger::error("Send: Player with id={} was not found anywhere.", fd);
  }

  p->try_flush();
}

void Server::disconnect(int fd) {
  auto weak_p = find_player(fd);
  auto p = weak_p.lock();
  if (!p) {
    Logger::error("Disconnect: Player with id={} was not found anywhere.", fd);
  }

  terminate_player(p);
}

int Server::count_players() const {
  int count = 0;

  for (const auto &p : unnamed_) {
    count++;
  }

  for (const auto &p : lobby_) {
    count++;
  }

  for (const auto &r : rooms_) {
    for (const auto &p : r->players()) {
      count++;
    }
  }

  return count;
}

void Server::terminate_player(std::shared_ptr<Player> p) {
  remove_from_game_server(p);
  Logger::info("Player {}, fd={}, removed from the whole game.", p->nick(),
               p->fd());
  close_connection(p->fd());
}

void Server::close_connection(int fd) {
  // remove from epoll
  auto res = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
  Logger::info("fd={}, removed from epoll.", fd);
  if (res == -1) {
    Logger::error("Error when removing player from epoll: {}",
                  std::strerror(errno));
  }

  // close connection
  close(fd);
  Logger::info("Closed connection fd={}.", fd);
}

void Server::remove_from_game_server(std::shared_ptr<Player> p) {
  // find where the player is & lock them down
  auto location = where_player(p);

  // find players owning vector
  std::reference_wrapper<std::vector<std::shared_ptr<Player>>> owner = unnamed_;
  switch (location.state_) {
  case Player_State::NON_EXISTING: // should not happen
    Logger::error("{} have no related socekt on the server", Logger::more(p));
    return;
  case Player_State::UNNAMED:
    owner = unnamed_;
    break;
  case Player_State::LOBBY:
    owner = lobby_;
    break;
  case Player_State::ROOM:
  case Player_State::GAME:
    auto room = location.room_.lock();
    if (!room) {
      Logger::error("{} is in not-existing room.", Logger::more(p));
      return;
    }

    try {
      // move player from room to lobby
      broadcast_to_room(room, Protocol::DEAD(p), {p->fd()});
      leave_room(p, room);
      owner = lobby_;

    } catch (const std::exception &ex) {
      Logger::error("{} Error: {}", Logger::more(p), ex.what());
      // do nothing, because the player is still being terminated & is not in
      // the room, simply erased by chance
      return;
    }
  }

  // delete player from any owning vector
  auto &vec = owner.get();
  auto fd = p->fd();
  erase_by_fd(vec, fd);
}

std::vector<std::shared_ptr<Player>> Server::list_players() {
  std::vector<std::shared_ptr<Player>> result;
  result.reserve(max_clients_);

  result.insert(result.end(), unnamed_.begin(), unnamed_.end());
  result.insert(result.end(), lobby_.begin(), lobby_.end());

  for (auto &r : rooms_) {
    auto p = r->players();
    result.insert(result.end(), p.begin(), p.end());
  }

  return result;
}

std::weak_ptr<Player> Server::find_player(int fd) {
  auto all = list_players();

  auto it = std::find_if(
      all.begin(), all.end(),
      [fd](const std::shared_ptr<Player> &p) { return p->fd() == fd; });
  if (it == all.end()) {
    return {};
  }
  return *it;
}

std::weak_ptr<Player> Server::find_player(const std::string &nick) {
  auto all = list_players();

  auto it = std::find_if(
      all.begin(), all.end(),
      [&nick](const std::shared_ptr<Player> &p) { return p->nick() == nick; });
  if (it == all.end()) {
    return {};
  }
  return *it;
}

Player_Location Server::where_player(std::shared_ptr<Player> p) {
  Player_Location l;

  { // UNNAMED
    auto it = std::find_if(unnamed_.begin(), unnamed_.end(),
                           [&p](const std::shared_ptr<Player> &pp) {
                             return pp->fd() == p->fd();
                           });
    if (it != unnamed_.end()) {
      l.state_ = Player_State::UNNAMED;
      return l;
    }
  }

  { // LOBBY
    auto it = std::find_if(lobby_.begin(), lobby_.end(),
                           [&p](const std::shared_ptr<Player> &pp) {
                             return pp->fd() == p->fd();
                           });
    if (it != lobby_.end()) {
      l.state_ = Player_State::LOBBY;
      return l;
    }
  }

  { // ROOM/GAME
    for (const auto &r : rooms_) {
      auto players = r->players();
      auto it = std::find_if(players.begin(), players.end(),
                             [&p](const std::shared_ptr<Player> &pp) {
                               return pp->fd() == p->fd();
                             });
      if (it == players.end()) {
        continue;
      }

      // found room, what is its state
      l.room_ = r;
      switch (r->state()) {
      case Room_State::OPEN:
        l.state_ = Player_State::ROOM;
        break;
      case Room_State::PLAYING:
      case Room_State::FINISHED:
        l.state_ = Player_State::GAME;
        break;
      }

      return l;
    }
  }

  // not found
  Logger::error("Where-Player: Player not found anywhere on server.");
  l.state_ = Player_State::NON_EXISTING;
  return l;
}

void Server::maybe_ping(std::shared_ptr<Player> p) {
  auto ping_diff = std::chrono::steady_clock::now() - p->get_last_ping();
  auto ping_diff_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(ping_diff).count();

  if (ping_diff_ms > ping_timeout_ms_) {
    p->append_msg(Protocol::PING());
    p->set_last_ping();
  }
}

void Server::check_pong(std::shared_ptr<Player> p) {
  // when was the last PONG received
  auto pong_diff = std::chrono::steady_clock::now() - p->get_last_pong();
  auto pong_diff_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(pong_diff).count();

  // long inactivity
  if (pong_diff_ms > death_timeout_ms_) {
    Logger::error("Terminating player fd={}: didn't respond for {} seconds.",
                  p->fd(), pong_diff_ms / 1000);
    terminate_player(p);

    // short inactivity
  } else if (pong_diff_ms > sleep_timeout_ms_) {
    // how many sleeps did we missed already
    int n_sleeps = pong_diff_ms / sleep_timeout_ms_;
    bool new_sleep = n_sleeps > p->did_sleep_times();

    // only do this periodically on sleep timeout multipliers
    if (new_sleep) {
      // only notify room once
      if (p->did_sleep_times() == 0) {
        auto loc = where_player(p);
        auto room = loc.room_.lock();
        if (room) {
          broadcast_to_room(room, Protocol::SLEEP(p), {p->fd()});
        }
      }

      p->did_sleep_times(n_sleeps);
      Logger::warn("Player fd={} didn't respond for {} seconds.", p->fd(),
                   pong_diff_ms / 1000);
    }
  }
}

void Server::enable_sending(int fd) {
  set_epoll_events(fd, EPOLLIN | EPOLLOUT);
}

void Server::disable_sending(int fd) { set_epoll_events(fd, EPOLLIN); }

void Server::process_message(const std::vector<std::string> &msg,
                             std::shared_ptr<Player> p) {
  if (msg.size() < 1) {
    return;
  }

  // the first part of msg is command
  const auto &cmd = msg[0];
  auto it = handlers_.find(cmd);

  // find & execute command
  if (it != handlers_.end()) {
    auto fn = it->second;
    (this->*fn)(msg, p);

    // unknown command = player ends
    // every OK message is not invalid
  } else if (cmd != "OK") {
    terminate_player(p);
  }
}

void Server::handle_pong(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid PONG", Logger::more(p));
    terminate_player(p);
    return;
  }

  p->set_last_pong();
}

void Server::handle_name(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 2) {
    Logger::error("{} Invalid NAME, number of words", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto location = where_player(p);
  if (location.state_ != Player_State::UNNAMED) {
    Logger::error("{} Invalid NAME, player wasn't unnamed", Logger::more(p));
    terminate_player(p);
    return;
  }

  // RECONNECT strategy
  auto weak_existing = find_player(msg[1]);
  auto existing = weak_existing.lock();

  // this is a new player
  if (!existing) {
    p->nick(msg[1]);
    p->append_msg(Protocol::OK_NAME());

    move_player_by_fd(p->fd(), unnamed_, lobby_);
    Logger::info("{} have name and is in lobby.", Logger::more(p));

    // this is an existing player
  } else {
    // switch socket FD
    int old_fd = existing->fd();

    existing->fd(p->fd());
    existing->append_msg(Protocol::OK_NAME());

    // erase this temporary player object
    erase_by_fd(unnamed_, p->fd());
    close_connection(old_fd);

    Logger::info("Existing player name={} switched sockets: {} => {}",
                 existing->nick(), old_fd, existing->fd());
  }
}

void Server::handle_list_rooms(const std::vector<std::string> &msg,
                               std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid LIST_ROOMS", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::LOBBY) {
    Logger::info("{} tried to list rooms while not in lobby, disconnecting.",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  p->append_msg(Protocol::ROOMS(rooms_));
  Logger::info("{} listed rooms", Logger::more(p));
}

void Server::handle_join_room(const std::vector<std::string> &msg,
                              std::shared_ptr<Player> p) {
  if (msg.size() != 2) {
    Logger::error("{} Invalid JOIN_ROOM", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::LOBBY) {
    Logger::info("{} tried to join room while not in lobby, disconnecting.",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  int r_id = std::stoi(msg[1]);

  auto room_it =
      std::find_if(rooms_.begin(), rooms_.end(),
                   [r_id](const auto &r) { return r->id() == r_id; });
  if (room_it == rooms_.end()) { // cannot find room
    p->append_msg(Protocol::FAIL_JOIN_ROOM());
    Logger::info("{} couldn't join non-existing room.", Logger::more(p));
    return;
  }

  std::shared_ptr<Room> room = *room_it;

  if (room->state() != Room_State::OPEN) { // room full
    p->append_msg(Protocol::FAIL_JOIN_ROOM());
    Logger::info("{} couldn't join full room id={}.", Logger::more(p),
                 room->id());
    return;
  }

  // move to room & remove from lobby
  move_player_by_fd(p->fd(), lobby_, room->players());
  p->append_msg(Protocol::OK_JOIN_ROOM());
  broadcast_to_room(room, Protocol::JOIN(p), {p->fd()});

  Logger::info("{} joined room id={}.", Logger::more(p), room->id());

  // start game ==> server takes over control
  if (room->should_begin_game(players_in_game_)) {
    room->state(Room_State::PLAYING);
    broadcast_to_room(room, Protocol::GAME_START(), {});

    room->setup_game();

    // show everyone hand
    for (auto p : room->players()) {
      p->append_msg(Protocol::HAND(p));
    }

    // show everyone turn
    broadcast_to_room(room, Protocol::TURN(room->current_turn()), {});
  }
}

void Server::handle_create_room(const std::vector<std::string> &msg,
                                std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid CREATE_ROOM", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::LOBBY) {
    Logger::info("{} tried to create room while not in lobby, disconnecting.",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  if (rooms_.size() >= max_rooms_) { // already limit of rooms
    p->append_msg(Protocol::FAIL_CREATE_ROOM());
    Logger::info("{} Failed create new room - limit of rooms reached.",
                 Logger::more(p));
    return;
  }

  // create new room
  rooms_.emplace_back(std::make_shared<Room>());
  auto room = rooms_.back();
  Logger::info("{} New room id={} was created and joined", Logger::more(p),
               room->id());

  // move to room & remove from lobby
  move_player_by_fd(p->fd(), lobby_, room->players());
  p->append_msg(Protocol::OK_CREATE_ROOM());
}

void Server::handle_leave_room(const std::vector<std::string> &msg,
                               std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid LEAVE_ROOM", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::ROOM && loc.state_ != Player_State::GAME) {
    Logger::info("{} tried to leave room while not in one, disconnecting.",
                 Logger::more(p));

    terminate_player(p);
    return;
  }

  auto room = loc.room_.lock();
  if (!room) {
    Logger::warn("{} tried to leave non-existing room? disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  try {
    leave_room(p, room);
  } catch (const std::exception &ex) {
    Logger::error("Error: {}", ex.what());
    terminate_player(p);
  }
}

void Server::leave_room(std::shared_ptr<Player> p, std::shared_ptr<Room> r) {

  // move to lobby & remove from room
  // may throw
  move_player_by_fd(p->fd(), r->players(), lobby_);

  p->append_msg(Protocol::OK_LEAVE_ROOM());
  Logger::info("{} left room id={}.", Logger::more(p), r->id());

  // tell others in room
  broadcast_to_room(r, Protocol::LEAVE(p), {p->fd()});

  // remove empty room
  if (r->players().size() == 0) {
    rooms_.erase(
        std::find_if(rooms_.begin(), rooms_.end(),
                     [&r](const auto &rr) { return r->id() == rr->id(); }));
    Logger::info("Empty room id={} was closed.", r->id());

    // end game because someone left
  } else if (r->state() == Room_State::PLAYING) {
    broadcast_to_room(r, Protocol::WIN(), {});
    r->state(Room_State::FINISHED);
  }
}

void Server::handle_room_info(const std::vector<std::string> &msg,
                              std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid ROOM_INFO", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::ROOM && loc.state_ != Player_State::GAME) {
    Logger::info("{} tried to get room info while not in one, disconnecting.",
                 Logger::more(p));

    terminate_player(p);
    return;
  }

  auto room = loc.room_.lock();
  if (!room) {
    Logger::warn("{} tried to get info about non-existing room? disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  p->append_msg(Protocol::ROOM(room));
  Logger::info("{} sent room info.", Logger::more(p));
}

void Server::handle_state(const std::vector<std::string> &msg,
                          std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid STATE", Logger::more(p));
    terminate_player(p);
    return;
  }

  p->append_msg(Protocol::STATE(*this, p));
  Logger::info("{} sent state.", Logger::more(p));
}

void Server::handle_play(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 2) {
    Logger::error("{} Invalid PLAY", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::GAME) {
    Logger::info(
        "{} tried to play card when not in game state => disconnecting.",
        Logger::more(p));

    terminate_player(p);
    return;
  }

  auto room = loc.room_.lock();
  if (!room) {
    Logger::warn("{} tried to play in non-existing room? disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  if (room->state() != Room_State::PLAYING) {
    Logger::warn("{} tried to play in room which is not playing, disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  if (room->current_turn().name_ != p->nick()) {
    Logger::warn("{} tried to play when not on turn, disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  Card c{msg[1][0], msg[1][1]};

  if (!c.is_valid()) {
    Logger::warn("{} tried to play invalid card ({}), disconnecting",
                 Logger::more(p), c.to_string());
    terminate_player(p);
    return;
  }

  if (!room->play_card(c)) {
    Logger::warn(
        "{} tried to play card which cannot be played ({}), disconnecting",
        Logger::more(p), c.to_string());
    terminate_player(p);
    return;
  }

  p->append_msg(Protocol::OK_PLAY());
  broadcast_to_room(room, Protocol::PLAYED(p, c), {p->fd()});

  Logger::info("{} played card={}", Logger::more(p), c.to_string());

  // is this end of game?
  auto w = room->get_winner();
  auto win = w.lock();
  if (win) {
    win->append_msg(Protocol::WIN());
    broadcast_to_room(room, Protocol::LOSE(), {win->fd()});
    room->state(Room_State::FINISHED);

    // return control to clients
    return;
  }

  if (c.rank_ == 'A') {
    // next player = is theoretically current, because play_card advanced
    auto np = room->current_player();
    broadcast_to_room(room, Protocol::SKIP(np), {});
    // really skip the player
    room->advance_player();

  } else if (c.rank_ == '7') {
    auto np = room->current_player();
    // draw cards
    auto c1 = room->deal_card();
    auto c2 = room->deal_card();

    // give them to player
    auto &hand = np->hand();
    hand.push_back(c1);
    hand.push_back(c2);

    // send it to people
    np->append_msg(Protocol::CARDS({c1, c2}));
    broadcast_to_room(room, Protocol::DRAWED(np, 2), {np->fd()});
    // skip the player
    room->advance_player();
  }

  // next turn
  broadcast_to_room(room, Protocol::TURN(room->current_turn()), {});
}

void Server::handle_draw(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{} Invalid DRAW", Logger::more(p));
    terminate_player(p);
    return;
  }

  auto loc = where_player(p);
  if (loc.state_ != Player_State::GAME) {
    Logger::info(
        "{} tried to draw card when not in game state => disconnecting.",
        Logger::more(p));

    terminate_player(p);
    return;
  }

  auto room = loc.room_.lock();
  if (!room) {
    Logger::warn("{} tried to draw in non-existing room? disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  if (room->state() != Room_State::PLAYING) {
    Logger::warn("{} tried to draw in room which is not playing, disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  if (room->current_turn().name_ != p->nick()) {
    Logger::warn("{} tried to draw when not on turn, disconnecting",
                 Logger::more(p));
    terminate_player(p);
    return;
  }

  // draw card
  auto c = room->deal_card();

  // give them to player
  auto &hand = p->hand();
  hand.push_back(c);

  // send it to people
  p->append_msg(Protocol::CARDS({c}));
  broadcast_to_room(room, Protocol::DRAWED(p, 1), {p->fd()});

  // next turn
  room->advance_player();
  broadcast_to_room(room, Protocol::TURN(room->current_turn()), {});
}

void Server::move_player_by_fd(int fd,
                               std::vector<std::shared_ptr<Player>> &from,
                               std::vector<std::shared_ptr<Player>> &to) {
  move_player([fd](const auto &p) { return p->fd() == fd; }, from, to);
}
void Server::move_player_by_nick(const std::string &nick,
                                 std::vector<std::shared_ptr<Player>> &from,
                                 std::vector<std::shared_ptr<Player>> &to) {
  move_player([&nick](const auto &p) { return p->nick() == nick; }, from, to);
}

void Server::broadcast_to_room(std::shared_ptr<Room> r, const std::string &msg,
                               const std::vector<int> &except_fds) {
  // for every player
  for (auto &p : r->players()) {
    // look if isn't in except vector
    auto here = std::find(except_fds.begin(), except_fds.end(), p->fd());
    // isn't => send message
    if (here == except_fds.end()) {
      p->append_msg(msg);
    }
  }
}

} // namespace prsi
