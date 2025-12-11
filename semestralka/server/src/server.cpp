#include "server.hpp"
#include "config.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace prsi {

Server::Server(const Config &cfg)
    : port_(cfg.port_), epoll_max_events_(cfg.epoll_max_events_),
      epoll_timeout_ms_(cfg.epoll_timeout_ms_), max_clients_(cfg.max_clients_),
      ping_timeout_ms_(cfg.ping_timeout_ms_),
      sleep_timeout_ms_(cfg.sleep_timeout_ms_),
      death_timeout_ms_(cfg.death_timeout_ms_) {
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

  // bind
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) != 0) {
    throw std::runtime_error("Cannot bind listen socket.");
  }

  if (listen(listen_fd_, SOMAXCONN) == -1) {
    throw std::runtime_error("Cannot listen.");
  }

  epoll_fd_ = epoll_create(0);

  if (set_epoll_events(listen_fd_, EPOLLIN, true) == -1) {
    throw std::runtime_error("Cannot add listening socket to epoll.");
  }
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

} // namespace prsi
