#include <algorithm>
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client_details.hpp"
#include "server.hpp"

volatile sig_atomic_t Server::running = 1;

void Server::handle_sigint(int) {
  running = 0;
  std::cout << std::endl
            << "Shutting down: Interrupt caused by the user." << std::endl;
}

Server::~Server() {
  for (auto &pair : clients) {
    close(pair.first);
  }
  clients.clear();

  if (listen_fd >= 0) {
    close(listen_fd);
    listen_fd = -1;
  }

  fds.clear();
}

int Server::create_listening_socket() {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    if (mVerbose)
      perror("socket");
    return 1;
  }

  int optval = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  sockaddr_in addr{AF_INET, htons(mPort), {INADDR_ANY}};
  if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    if (mVerbose)
      perror("bind");
    close(listen_fd);
    return 1;
  }

  if (listen(listen_fd, SOMAXCONN) < 0) {
    if (mVerbose)
      perror("listen");
    close(listen_fd);
    return 1;
  }

  fds.push_back({listen_fd, POLLIN, 0});
  if (mVerbose)
    std::cout << "Server listening on port " << mPort << std::endl;
  return listen_fd;
}

void Server::accept_new_client() {
  sockaddr_in client_addr{};
  socklen_t len = sizeof(client_addr);
  int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &len);
  if (client_fd < 0) {
    if (mVerbose)
      perror("accept");
    return;
  }

  fds.push_back({client_fd, POLLIN, 0});
  clients[client_fd] = Client_Details{};
  if (mVerbose)
    std::cout << "Client connected: fd=" << client_fd << std::endl;
}

bool Server::handle_client_event(const int fd) {
  char buf[1024];
  ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
  if (n <= 0)
    return false;

  buf[n] = '\0';
  auto &client = clients[fd];
  std::string s = client.handle_message(buf);

  send(fd, s);

  return client.is_active();
}

void Server::remove_client(const int fd) {
  if (mVerbose)
    std::cout << "Removing client fd=" << fd << std::endl;
  close(fd);
  clients.erase(fd);
  fds.erase(std::remove_if(fds.begin(), fds.end(),
                           [fd](const pollfd &p) { return p.fd == fd; }),
            fds.end());
}

void Server::send(const int fd, const std::string &message) {
  ssize_t bytes_sent = ::send(fd, message.c_str(), message.size(), 0);
  if (bytes_sent <= 0 && mVerbose)
    perror("send");
}

void Server::run() {
  if (create_listening_socket() < 0)
    return;

  while (running) {
    int ret = poll(fds.data(), fds.size(), mTimeout);
    if (ret < 0) {
      if (mVerbose)
        perror("poll");
      break;
    }

    for (size_t i = 0; i < fds.size(); ++i) {
      short rev = fds[i].revents;
      if (!rev)
        continue; // Skip if nothing is going on

      if (rev & (POLLERR | POLLHUP | POLLNVAL)) { // error occurred
        remove_client(fds[i].fd);
        --i;
        continue;
      }

      if (rev & POLLIN) {             // ready to accept/sent something
        if (fds[i].fd == listen_fd) { // new client
          accept_new_client();
          continue;
        }
        if (!handle_client_event(fds[i].fd)) { // sent something
          remove_client(fds[i].fd); // If after handling should be terminated
          --i;
        }
      }
    }
  }

  if (mVerbose)
    std::cout << "Stopping the server listening on port: "
              << std::to_string(mPort) << std::endl;
}
