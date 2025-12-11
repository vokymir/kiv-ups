#include "player.hpp"
#include "logger.hpp"
#include <cerrno>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>

namespace prsi {

Player::Player(int fd) : fd_(fd) {}

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

    last_received();
  }
}

void Player::send(std::string &msg) {}

void Player::flush() {}

} // namespace prsi
