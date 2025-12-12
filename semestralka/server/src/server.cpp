#include "server.hpp"
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
      death_timeout_ms_(cfg.death_timeout_ms_), ip_(cfg.ip_) {

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
      Logger::info("message size: {}", msg.size());
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
  case NON_EXISTING: // should not happen
    Logger::error(
        "There is player with no related socekt on the server with id={}",
        p->fd());
    return;
  case UNNAMED:
    owner = unnamed_;
    break;
  case LOBBY:
    owner = lobby_;
    break;
  case ROOM:
  case GAME:
    // TODO: broadcast to other players in the room, end game or whatever, close
    // room, etc

    auto ownr = location.room_.lock();
    if (!ownr) {
      Logger::error("Room disappeared quickly.");
      return;
    }
    owner = ownr->players();
    break;
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
      l.state_ = UNNAMED;
      return l;
    }
  }

  { // LOBBY
    auto it = std::find_if(lobby_.begin(), lobby_.end(),
                           [&p](const std::shared_ptr<Player> &pp) {
                             return pp->fd() == p->fd();
                           });
    if (it != lobby_.end()) {
      l.state_ = LOBBY;
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
      case OPEN:
        l.state_ = ROOM;
        break;
      case PLAYING:
      case FINISHED:
        l.state_ = GAME;
        break;
      }

      return l;
    }
  }

  // not found
  Logger::error("Where-Player: Player not found anywhere on server.");
  l.state_ = NON_EXISTING;
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
        Logger::info("notify room that fd={} is sleeping", p->fd());
        // TODO: if is in room, notify the room - multiple times even
      }

      p->sleep_intensity(n_sleeps);
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
  } else {
    terminate_player(p);
  }
}

void Server::handle_pong(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{}", Logger::more("Invalid PONG", p));
    terminate_player(p);
    return;
  }

  p->set_last_pong();
}

void Server::handle_name(const std::vector<std::string> &msg,
                         std::shared_ptr<Player> p) {
  if (msg.size() != 2) {
    Logger::error("{}", Logger::more("Invalid NAME, number of words", p));
    terminate_player(p);
    return;
  }

  auto location = where_player(p);
  if (location.state_ != UNNAMED) {
    Logger::error("{}", Logger::more("Invalid NAME, player isn't unnamed", p));
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

    // this is an existing player
  } else {
    // switch socket FD
    int old_fd = existing->fd();

    existing->fd(p->fd());
    existing->append_msg(Protocol::OK_NAME());

    // erase this temporary player object
    erase_by_fd(unnamed_, p->fd());
    close_connection(old_fd);
  }
}

void Server::handle_list_rooms(const std::vector<std::string> &msg,
                               std::shared_ptr<Player> p) {
  if (msg.size() != 1) {
    Logger::error("{}", Logger::more("Invalid LIST_ROOMS", p));
    terminate_player(p);
    return;
  }

  p->append_msg(Protocol::ROOMS(rooms_));
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

} // namespace prsi
