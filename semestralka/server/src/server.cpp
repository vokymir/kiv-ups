#include "server.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "player.hpp"
#include "room.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace prsi {

Server::~Server() {
  // close all connections
  // close listen socket
  // close epoll socket
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
    // wait for n events to happen, n it at most epoll_max_events_
    int n = epoll_wait(epoll_fd_, events_.data(), epoll_max_events_,
                       epoll_timeout_ms_);

    if (n == -1) {
      if (errno == EINTR) {
        continue; // ignore interrupts
      } // report anything else
      throw std::runtime_error("epoll_wait failed.");
    }

    for (int i = 0; i < n; i++) {
      epoll_event &ev = events_[i];

      if (ev.data.fd == listen_fd_) { // NEW CONNECTION
        accept_connection();
      } else if (ev.events & EPOLLIN) {               // RECV
      } else if (ev.events & EPOLLOUT) {              // SEND
      } else if (ev.events & (EPOLLHUP | EPOLLERR)) { // DISCONNECT
        auto weak_p = find_player(ev.data.fd);
        auto p = weak_p.lock();
        if (!p) {
          Logger::error("Player with id={} was not found anywhere.",
                        std::to_string(ev.data.fd));
        }
        terminate_player(p);
      }
    }

    // dispatch all out_evs in GM.out()
    // and send them
    // check for timeouts
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

  if (listen(listen_fd_, SOMAXCONN) == -1) {
    throw std::runtime_error("Cannot listen.");
  }

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
  auto player = std::make_shared<Player>(client_fd);
  unnamed_.emplace_back(player);

  Logger::info("New client connected, fd={}", client_fd);
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
  remove_from_game(p);
  Logger::info("Player {}, fd={}, removed from game.", p->nick(), p->fd());

  // remove from epoll
  auto res = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, p->fd(), nullptr);
  Logger::info("Player {}, fd={}, removed from epoll.", p->nick(), p->fd());
  if (res == -1) {
    Logger::error("Error when removing player from epoll: {}",
                  std::strerror(errno));
  }

  // close connection
  close(p->fd());
  Logger::info("Closed connection fd={}.", p->fd());
}
void Server::remove_from_game(std::shared_ptr<Player> p) {
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
    // find room in rooms
    auto it = std::find_if(rooms_.begin(), rooms_.end(),
                           [location](const std::shared_ptr<Room> r) {
                             return r->id() == location.room_id_;
                           });
    if (it == rooms_.end()) {
      Logger::warn("Tried to remove player from room {}, but there is no room "
                   "with that id.",
                   location.room_id_);
      return;
    }
    // TODO: broadcast to other players in the room, end game or whatever, close
    // room, etc

    owner = (*it)->players();
    break;
  }

  // delete player from any owning vector
  auto vec = owner.get();
  auto it = std::find(vec.begin(), vec.end(), p);
  if (it != vec.end()) {
    vec.erase(it);
  }
}

std::vector<std::shared_ptr<Player>> Server::list_players() {
  std::vector<std::shared_ptr<Player>> result;
  result.reserve(max_clients_);

  result.insert(unnamed_.begin(), unnamed_.end(), result.end());
  result.insert(lobby_.begin(), lobby_.end(), result.end());

  for (auto &r : rooms_) {
    auto p = r->players();
    result.insert(p.begin(), p.end(), result.end());
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
      if (it == lobby_.end()) {
        continue;
      }

      // found room, what is its state
      l.room_id_ = r->id();
      switch (r->state()) {
      case OPEN:
      case FULL:
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
  Logger::error("Player not found anywhere on server.");
  l.state_ = NON_EXISTING;
  return l;
}

} // namespace prsi
