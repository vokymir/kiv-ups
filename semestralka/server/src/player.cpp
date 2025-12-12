#include "player.hpp"
#include "logger.hpp"
#include <cerrno>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>

namespace prsi {

Player::Player(int fd) : fd_(fd) {
  set_last_received();
  set_last_ping();
}

Player::~Player() {}

void Player::receive() {
  char buff[1024];
  int flags = 0;

  while (true) {
    ssize_t n = recv(fd_, buff, sizeof(buff), flags);

    if (n == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      Logger::error("recv failed for fd={}", fd_);
      throw std::runtime_error("Failed receive, should disconnect.");
    }

    if (n == 0) { // client closed connection
      throw std::runtime_error("Client closed connection.");
    }

    read_buffer_.append(buff, n);
    if (read_buffer_.size() > 1'000'000) {
      throw std::runtime_error("Too long message buffer, probably an attack.");
    }

    Logger::info("Received {} bytes from fd={}", n, fd_);
    set_last_received();
    Logger::info("fd={} have in recv buffer: '{}'", fd_,
                 read_buffer_); // TODO: remove
  }
}

void Player::send(const std::string &msg) { write_buffer_.append(msg); }

void Player::flush() {}

} // namespace prsi
