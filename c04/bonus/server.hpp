#pragma once

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <csignal>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "client_details.hpp"

class Server {
public:
  int mPort = 10000;
  int mTimeout = 15000;
  bool mVerbose = true;

  Server(int port = 10000, int timeout = 15000, bool verbose = true)
      : mPort(port), mTimeout(timeout), mVerbose(verbose) {}
  ~Server();

  /*Create socket, then in loop accept&handle clients.*/
  void run();
  static void handle_sigint(int);

private:
  /*Return FD of socket.*/
  int create_listening_socket();
  void accept_new_client();
  /*Return true if everything is OK.
   * Return false if client should be removed.*/
  bool handle_client_event(const int fd);
  void send(const int fd, const std::string &message);
  void remove_client(const int fd);

private:
  int listen_fd = -1; // socket FD
  std::vector<pollfd> fds;
  std::unordered_map<int, Client_Details> clients; // mapped on pollfd.fd
  static volatile sig_atomic_t running;
};
