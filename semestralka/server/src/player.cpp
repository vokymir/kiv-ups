#include "player.hpp"
#include "logger.hpp"
#include "protocol.hpp"
#include "server.hpp"
#include <algorithm>
#include <cerrno>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>

namespace prsi {

Player::Player(Server &s, int fd) : server_(s), fd_(fd) {
  this->fd(fd_); // to set fd valid
  set_last_pong();
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
  }
}

void Player::append_msg(const std::string &msg) {
  write_buffer_.append(msg);
  try_flush();
}

void Player::try_flush() {
  ssize_t sent = send(fd_, write_buffer_.data(), write_buffer_.size(), 0);

  if (sent > 0) { // success
    write_buffer_.erase(0, sent);

    if (write_buffer_.empty()) { // writen everything
      server_.disable_sending(fd_);
    } else { // something needs to be retried
      server_.enable_sending(fd_);
    }

    // socket is working, but dont have time or what
  } else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    server_.enable_sending(fd_); // retry next time

    // socket is not a friend anymore
  } else {
    server_.terminate_player(std::make_shared<Player>(*this));
  }
}

std::vector<std::string> Player::complete_recv_msg() {
  if (!Protocol::could_validate(read_buffer_)) {
    return {};
  }

  if (!Protocol::valid(read_buffer_)) {
    throw std::runtime_error("Not a valid protocol message.");
  }

  return Protocol::extract_message(read_buffer_);
}
void Player::set_last_pong(std::chrono::steady_clock::time_point time) {
  if (did_sleep_times_ > 0) {
    // if is in room, tell others that now i am awake
    auto p = shared_from_this();
    auto loc = server_.where_player(p);
    auto r = loc.room_.lock();
    if (r) {
      server_.broadcast_to_room(r, Protocol::AWAKE(p), {fd_});
    }
  }
  did_sleep_times_ = 0;
  last_pong_ = time;
};

bool Player::have_card(const Card &c) {
  auto it = std::find_if(hand_.begin(), hand_.end(), [&c](const auto &card) {
    return c.rank_ == card.rank_ && c.suit_ == card.suit_;
  });

  return it != hand_.end();
}

void Player::remove_card(const Card &c) {
  auto it = std::find_if(hand_.begin(), hand_.end(), [&c](const auto &card) {
    return c.rank_ == card.rank_ && c.suit_ == card.suit_;
  });
  if (it == hand_.end()) {
    return; // cannot find
  }

  hand_.erase(it);
}

void Player::clear_hand() { hand_.resize(0); }

} // namespace prsi
