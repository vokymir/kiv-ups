#include "server/Client.hpp"
#include "game/Room.hpp"
#include "server/Message.hpp"
#include "server/Protocol.hpp"
#include "server/Server.hpp"
#include "util/Logger.hpp"
#include <chrono>
#include <cstddef>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace prsi::server {

int Client::fd() const { return fd_; }

void Client::set_nickname(const std::string &nick) {
  if (!nick.empty()) {
    nickname_ = nick;
  }
}
const std::string &Client::nickname() const { return nickname_; }

std::string &Client::read_buffer() { return read_buffer_; }
std::string &Client::write_buffer() { return write_buffer_; }

void Client::set_current_room(game::Room *room) {
  if (current_room_ != nullptr) {
    throw std::runtime_error("Cannot join room until leaving current one.");
  }
  current_room_ = room;
}
game::Room *Client::current_room() const { return current_room_; }

void Client::set_state(Client_State state) { state_ = state; }
Client_State Client::state() const { return state_; }

void Client::set_last_received(
    const std::chrono::steady_clock::time_point &when) {
  if (when > last_received_) // allow only more recent times
    last_received_ = when;
}
void Client::set_last_received_now() {
  last_received_ = std::chrono::steady_clock::now();
}
const std::chrono::steady_clock::time_point Client::last_received() const {
  return last_received_;
}

void Client::set_last_sent(const std::chrono::steady_clock::time_point &when) {
  if (when > last_sent_) // allow only more recent times
    last_sent_ = when;
}
void Client::set_last_sent_now() {
  last_sent_ = std::chrono::steady_clock::now();
}
const std::chrono::steady_clock::time_point Client::last_sent() const {
  return last_sent_;
}

void Client::process_complete_messages(Server &server) {
  while (auto message = extract_next_message()) {
    set_last_received_now();

    try {
      Client_Message msg = Protocol::parse(*message);
      message_handler(server, msg);
    } catch (const std::exception &e) {
      server.handle_client_disconnect(fd_);
      return;
    }
  }
}

std::optional<std::string> Client::extract_next_message() {
  size_t pos = read_buffer_.find('\n');
  if (pos == std::string::npos) {
    return std::nullopt;
  }

  std::string message = read_buffer_.substr(0, pos);
  read_buffer_.erase(0, pos + 1); // remove message + newline
  return message;
}

void Client::message_handler(Server &server, const Client_Message &msg) {
  std::visit(
      [&](auto &&m) {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, CM_Pong>) {
          // something
        } else {
          util::Logger::error(
              "Client sent invalid message, disconnecting fd={}, nickname={}",
              fd_, nickname_);
          server.handle_client_disconnect(fd_);
        }
      },
      msg);
}

} // namespace prsi::server
