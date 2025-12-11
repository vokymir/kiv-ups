#include "server.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "player.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <fcntl.h>
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

  events_.reserve(epoll_max_events_);
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

} // namespace prsi
