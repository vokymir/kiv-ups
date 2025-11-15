#include "server/Server.hpp"
#include "util/Config.hpp"
#include "util/Logger.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <exception>
#include <fcntl.h>
#include <fmt/format.h>
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

void Server::accept_connection() {
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);

  int client_fd = accept(listen_fd_, (sockaddr *)&client_addr, &addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    util::Logger::error("accept() failed");
    return;
  }

  // do we have space for new connection?
  if (clients_.size() >= cfg_.max_clients()) {
    close(client_fd);
    util::Logger::warn("Max clients reached, rejecting connection");
    return;
  }

  // setup client socket
  int flags = fcntl(client_fd, F_GETFL, 0);
  if (flags == -1)
    throw std::runtime_error("fcntl F_GETFL failed");
  if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    throw std::runtime_error("fcntl F_SETFL failed");

  // add to epoll
  epoll_event ev{};
  ev.events = EPOLLIN; // read from client
  ev.data.fd = client_fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
    close(client_fd);
    util::Logger::error("Cannot add client to epoll, closing connection");
    return;
  }

  // create new client
  clients_.emplace(client_fd, Client(client_fd));
  util::Logger::info(::fmt::format("New client connected, fd={}", client_fd));
}

void Server::handle_client_data(int fd) {
  auto it = clients_.find(fd);
  if (it == clients_.end()) {
    util::Logger::error(
        ::fmt::format("Client not found (shouldn't happen :( ), fd={})", fd));
    return;
  }

  Client &client = it->second;

  // TODO: magic number
  char buffer[4096]; // temporary buffer for client read
  ssize_t n = read(fd, buffer, sizeof(buffer));

  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    util::Logger::error(
        ::fmt::format("Read error, disconnect client, fd={}", fd));
    handle_clent_disconnect(fd);
    return;
  }

  if (n == 0) { // client closed connection
    handle_clent_disconnect(fd);
    return;
  }

  client.read_buffer_.append(buffer, static_cast<unsigned long>(n));
  client.last_activity_ = std::chrono::steady_clock::now();

  try {
    client.process_complete_messages();
  } catch (std::exception e) {
    util::Logger::error(::fmt::format("Client process complete messages error "
                                      ": '{}', disconnect client, fd={}",
                                      e.what(), fd));
    handle_clent_disconnect(fd);
    return;
  }
}
