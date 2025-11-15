#include "server/Server.hpp"
#include "util/Config.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using prsi::server::Server;

Server::Server() {
  events_.reserve(static_cast<size_t>(cfg_.epoll_max_events()));
}

void Server::setup() {
  // create socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1)
    throw std::runtime_error("Cannot create listen socket.");

  // set socket options
  int opt = 1;
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    throw std::runtime_error("Cannot set socket options.");

  // set non-blocking
  int flags = fcntl(listen_fd_, F_GETFL, 0);
  if (flags == -1)
    throw std::runtime_error("fcntl F_GETFL failed");

  if (fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) == -1)
    throw std::runtime_error("fcntl F_SETFL failed");

  // bind
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(cfg_.port());
  if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) != 0)
    throw std::runtime_error("Cannot bind listen socket.");

  if (listen(listen_fd_, SOMAXCONN) == -1)
    throw std::runtime_error("Cannot listen.");

  epoll_fd_ = epoll_create1(0);

  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = listen_fd_;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) == -1)
    throw std::runtime_error("Cannot add listening to epoll.");
}

void Server::run() {
  running_ = true;
  while (running_) {
    // wait for <n> events to happen, <n> is at most spoll_max_events
    int n = epoll_wait(epoll_fd_, events_.data(), cfg_.epoll_max_events(),
                       cfg_.epoll_timeout_ms());

    if (n == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("epoll_wait failed");
    }

    // handle every event
    for (int i = 0; i < n; i++) {
      epoll_event &ev = events_[static_cast<size_t>(i)];

      if (ev.data.fd == listen_fd_) {
        accept_connection();
      } else if (ev.events & EPOLLIN) {
        handle_client_data(ev.data.fd);
      } else if (ev.events & (EPOLLHUP | EPOLLERR)) {
        handle_clent_disconnect(ev.data.fd);
      }
    }

    check_timeouts();
  }
}
