#pragma once

#include "prsi/game/room.hpp"
#include "prsi/net/string_protocol.hpp"
#include "prsi/util/config.hpp"
#include <chrono>
#include <memory>
#include <string>

namespace prsi::net {

// connection status
enum Session_Status {
  Awake,
  Sleep, // brief disconnect
  Dead,  // long disconnect
};

// Handle message recv/send buffers, pings.
// Use protocol.
class Session {
public:
  Session(const util::Config &config);
  ~Session() = default;

private:
  int fd_;
  std::string nick_;

  std::string read_buffer_;
  std::string write_buffer_;

  std::chrono::steady_clock::time_point last_recv_;
  std::chrono::steady_clock::time_point last_send_;

  // if is null -> session in lobby
  // not owner
  std::weak_ptr<game::Room> room_;

  // own its own protocol -> each session could use different
  std::unique_ptr<String_Protocol> protocol_;

  Session_State state_;
  Session_Status connection_status_;

public:
  // get/set
  const std::string &nick() { return nick_; }
  std::weak_ptr<game::Room> room() { return room_; }
  void room(std::weak_ptr<game::Room> room) { room_ = std::move(room); }

public:
  // networking

  // read incoming messages from fd
  void receive();

  // process complete messages from read_buffer_
  void process();

  // send what's inside write_buffer_ (if is anything)
  void send();
};

} // namespace prsi::net
