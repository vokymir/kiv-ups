#include "server/Client.hpp"
#include "game/Room.hpp"
#include <chrono>
#include <stdexcept>
#include <string>

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

/* TODO: in a moment, now as a reminder
 *
// Process complete messages (e.g., newline-delimited)
size_t pos;
while ((pos = client.read_buffer.find('\n')) != std::string::npos) {
  std::string message = client.read_buffer.substr(0, pos);
  client.read_buffer.erase(0, pos + 1);

  // Parse and handle the message
  try {
    process_message(fd, message);
  } catch (const std::exception &e) {
    client.invalid_msg_count++;
    if (client.invalid_msg_count >= 3) {
      handle_client_disconnect(fd);
      return;
    }
  }
}
*/

} // namespace prsi::server
