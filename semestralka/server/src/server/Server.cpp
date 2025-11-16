#include "server/Server.hpp"
#include "server/Client.hpp"
#include "server/Message.hpp"
#include "server/Protocol.hpp"
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
#include <string>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

namespace prsi::server {

Server::Server() {
  events_.reserve(static_cast<size_t>(cfg_.epoll_max_events()));
  setup();
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
        handle_client_read(ev.data.fd);
      } else if (ev.events & EPOLLOUT) {
        handle_client_write(ev.data.fd);
      } else if (ev.events & (EPOLLHUP | EPOLLERR)) {
        handle_client_disconnect(ev.data.fd);
      }
    }

    check_timeouts();
  }
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
  if (!set_fd_nonblocking(listen_fd_)) {
    throw std::runtime_error("Cannot set listen socket non-blocking.");
  }

  // bind
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(cfg_.port());
  if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) != 0)
    throw std::runtime_error("Cannot bind listen socket.");

  if (listen(listen_fd_, SOMAXCONN) == -1)
    throw std::runtime_error("Cannot listen.");

  epoll_fd_ = epoll_create1(0);

  if (!set_epoll_events(listen_fd_, EPOLLIN, true))
    throw std::runtime_error("Cannot add listening to epoll.");
}

bool Server::set_fd_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    return false;

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    return false;

  return true;
}

bool Server::set_epoll_events(int fd, uint32_t events, bool creating_new) {
  epoll_event ev{};
  ev.events = events;
  ev.data.fd = fd;

  if (epoll_ctl(epoll_fd_, creating_new ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, fd,
                &ev) == -1) {
    util::Logger::error("Failed to modify epoll events for fd={}", fd);
    return false;
  }
  return true;
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
  if (clients_.size() >= static_cast<size_t>(cfg_.max_clients())) {
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
  util::Logger::info("New client connected, fd={}", client_fd);
}

void Server::handle_client_read(int fd) {
  auto it = clients_.find(fd);
  if (it == clients_.end()) {
    util::Logger::error("Client not found (shouldn't happen :( ), fd={})", fd);
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
    util::Logger::error("Read error, disconnect client, fd={}", fd);
    handle_client_disconnect(fd);
    return;
  }

  if (n == 0) { // client closed connection
    handle_client_disconnect(fd);
    return;
  }

  client.read_buffer().append(buffer, static_cast<unsigned long>(n));
  client.set_last_received_now();

  try {
    client.process_complete_messages(*this);
  } catch (std::exception &e) {
    util::Logger::error("Client process complete messages error: '{}', "
                        "disconnect client, fd={}",
                        e.what(), fd);
    handle_client_disconnect(fd);
    return;
  }
}

void Server::handle_client_write(int fd) {
  auto it = clients_.find(fd);
  if (it == clients_.end()) {
    return; // clinet not found
  }
  Client &client = it->second;
  auto &buffer = client.write_buffer();

  if (buffer.empty()) { // nothing to write
    set_epoll_events(fd, EPOLLIN);
    return;
  }

  ssize_t n = write(fd, buffer.data(), buffer.size());

  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return; // socket not ready, try later
    }
    // write error - disconnect
    handle_client_disconnect(fd);
    return;
  }

  // remove sent data from buffer
  if (n > 0) {
    buffer.erase(0, static_cast<size_t>(n));
    client.set_last_sent_now();
  }

  // if all data sent
  if (buffer.empty()) {
    set_epoll_events(fd, EPOLLIN);
  }
  // if not, will send next chunk of data next time
}

void Server::handle_client_disconnect(int fd) {
  auto it = clients_.find(fd);
  if (it == clients_.end()) {
    util::Logger::warn(
        "Tried to disconnect already disconnected/non-existing client, fd={}",
        fd);
    return; // client already removed
  }

  Client &client = it->second;
  auto room = client.current_room();

  // if is in a room
  if (room != nullptr) {
    // notify others in room
    broadcast_to_room(room, SM_Someone_Disconnected{fd}, fd);

    // remove client from room
    room->remove_player(client.fd());

    // if room is empty or game cannot continue
    if (room->should_close()) {
      lobby_.remove_room(room->id());
    }
  }

  // remove from epoll and close connection
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
  close(fd);
  clients_.erase(it);
  util::Logger::info("Client disconnected, fd={}, nickname={}", fd,
                     client.nickname());
}

void Server::check_timeouts() {
  auto now = std::chrono::steady_clock::now();
  auto ping_time = std::chrono::seconds(cfg_.ping_timeout_s());
  auto disconnect_time = std::chrono::seconds(cfg_.disconnect_timeout_s());

  std::vector<int> to_disconnect;

  for (auto &[fd, client] : clients_) {
    auto inactive_time = now - client.last_received();

    if (inactive_time > disconnect_time) { // DISCONNECT
      to_disconnect.push_back(fd);
      util::Logger::warn(
          "Client timeout, fd={}, inactive for {} seconds", fd,
          std::chrono::duration_cast<std::chrono::seconds>(inactive_time)
              .count());
      continue;
    }

    auto last_try = now - client.last_sent();
    if (inactive_time > ping_time && last_try > ping_time) { // PING
      send_message(fd, SM_Ping{});
      client.set_last_sent_now();
    }
  }

  for (int fd : to_disconnect) {
    handle_client_disconnect(fd);
  }
}

void Server::send_message(int fd, const Server_Message &msg) {
  std::string serialized = Protocol::serialize(msg);

  auto it = clients_.find(fd);
  if (it == clients_.end()) {
    return; // client not found TODO: add warn maybe?
  }
  Client &client = it->second;

  client.write_buffer() += serialized;
  set_epoll_events(fd, EPOLLIN | EPOLLOUT);
}

void Server::broadcast_to_room(const prsi::game::Room *room,
                               const prsi::server::Server_Message &msg,
                               int except_fd) {
  if (room == nullptr) {
    return;
  }

  const auto &player_fds = room->get_player_fds();

  for (int fd : player_fds) {
    if (fd == except_fd) {
      continue;
    }
    send_message(fd, msg);
  }
}

} // namespace prsi::server
