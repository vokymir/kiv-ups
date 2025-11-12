#include "prsi/server/Server.hpp"
#include "prsi/game/Model.hpp"
#include "prsi/server/Session.hpp"
#include "prsi/util/Config.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace prsi::server {

Server::Server() {
  logger_.info("Server constructor called.");

  // Prepare structures
  config_ = std::make_unique<Config>(Config::instance());
  model_ = std::make_unique<Model>();

  // Create socket
  setup_server_socket();

  // Create epoll instance
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("Failed to create epoll: " +
                             std::string(strerror(errno)));
  }

  // Add server socket to epoll
  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = server_fd_;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) == -1) {
    throw std::runtime_error("Failed to add server socket to epoll.");
  }
}

Server::~Server() {
  logger_.info("Server destructor called.");

  for (auto &[fd, _] : sessions_) {
    close(fd);
  }
  sessions_.clear();

  if (server_fd_ != -1) {
    close(server_fd_);
  }
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
}

int Server::setup_server_socket() {
  server_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd_ < 0) {
    throw std::runtime_error("Failed to create server socket.");
  }

  // Set socket reuse address
  int opt = 1; // TRUE
  if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    close(server_fd_);
    throw std::runtime_error("Server failed to set socket SO_REUSEADDR.");
  }

  // Bind socket
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(config_->server_port);

  if (inet_pton(AF_INET, config_->server_address.c_str(), &addr.sin_addr) <=
      0) {
    close(server_fd_);
    throw std::runtime_error("Invalid address: " + config_->server_address);
  }

  if (bind(server_fd_, (sockaddr *)&addr, sizeof(addr)) < 0) {
    close(server_fd_);
    throw std::runtime_error("Server failed to bind: " +
                             std::string(strerror(errno)));
  }

  // Listen
  if (listen(server_fd_, SOMAXCONN) == -1) {
    close(server_fd_);
    throw std::runtime_error("Server failed to listen.");
  }

  return server_fd_;
}

void Server::process_events() {
  constexpr int MAX_EVENTS = 64;
  epoll_event events[MAX_EVENTS];

  int nfds =
      epoll_wait(epoll_fd_, events, MAX_EVENTS, config_->poll_timeout_ms);

  if (nfds == -1) {
    if (errno == EINTR) {
      return; // interrupted by signal
    }
    logger_.error("epoll_wait failed: " + std::string(strerror(errno)));
    return;
  }

  for (int i = 0; i < nfds; i++) {
    if (events[i].data.fd == server_fd_) {
      accept_new_connection();
    } else {
      handle_client_event(events[i].data.fd, events[i].events);
    }
  }

  auto now = std::chrono::steady_clock::now();
  std::vector<int> to_remove;

  for (auto &[fd, session] : sessions_) {
    if (/*session->is_timed_out*/ 1) {
      logger_.warn("Session <name.placeholder> timed out.");
      to_remove.push_back(fd);
    } else if (/*session->needs_ping*/ 1) {
      /*protocol::send_message(fd, "PING")*/
    }
  }

  for (int fd : to_remove) {
    remove_client(fd);
  }
}

void Server::accept_new_connection() {
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);

  int client_fd = accept(server_fd_, (sockaddr *)&client_addr, &addr_len);
  if (client_fd == -1) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      logger_.error("Accept failed: " + std::string(strerror(errno)));
    }
    return;
  }

  // check player limit
  if (sessions_.size() >= static_cast<size_t>(config_->max_players)) {
    /*protocol.send_msg(error|serverfull)*/
    close(client_fd);
    logger_.warn("Rejected connection: server full.");
    return;
  }

  // make client non-blocking
  int flags = fcntl(client_fd, F_GETFL, 0);
  if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    close(client_fd);
    logger_.error("Failed to set client non-blocking.");
    return;
  }

  // add to epoll
  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = client_fd;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
    close(client_fd);
    logger_.error("Failed to add client to epoll.");
    return;
  }

  // create client session
  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

  sessions_[client_fd] = std::make_unique<Session>();

  logger_.info(("New connection from " + std::string(client_ip) +
                " (fd= " + std::to_string(client_fd) + ")."));
}

void Server::handle_client_event(int fd, uint32_t events) {
  auto it = sessions_.find(fd);
  if (it == sessions_.end()) {
    return;
  }

  auto &session = it->second;

  if (events & (EPOLLERR | EPOLLHUP)) {
    logger_.info("Client <nickname> disconnected (error/hangup).");
    remove_client(fd);
    return;
  }

  if (events & EPOLLIN) {
    bool should_disconnect = false;

    try {
      should_disconnect = !session /*handle read()*/;
    } catch (const std::exception &e) {
      logger_.error("Exception handling client " + std::to_string(fd) + ": " +
                    e.what());
      should_disconnect = true;
    }

    if (should_disconnect) {
      remove_client(fd);
    }
  }
}

void Server::remove_client(int fd) {
  auto it = sessions_.find(fd);
  if (it == sessions_.end()) {
    return;
  }

  logger_.info("Removing client fd=" + std::to_string(fd));

  /*it->second->handle_disconnect() ???*/;

  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
  close(fd);
  sessions_.erase(it);
}

} // namespace prsi::server
