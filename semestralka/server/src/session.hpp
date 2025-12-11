#pragma once

#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include "prsi/mgr/room.hpp"
#include "prsi/net/string_protocol.hpp"
#include <chrono>
#include <deque>
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
  Session(int ping_timeout_ms, int sleep_timeout_ms, int death_timeout_ms);
  ~Session();

private:
  int fd_;
  std::string nick_;

  std::string read_buffer_;
  std::string write_buffer_;

  std::chrono::steady_clock::time_point last_recv_;
  std::chrono::steady_clock::time_point last_send_;

  int ping_timeout_ms_;
  int sleep_timeout_ms_;
  int death_timeout_ms_;

  // if is null -> session in lobby
  // not owner
  std::weak_ptr<mgr::Room> room_;

  // own its own protocol -> each session could use different
  std::unique_ptr<String_Protocol> protocol_;

  Session_State state_;

  bool should_disconnect_ = false;

public:
  // get/set
  const std::string &nick() const { return nick_; }
  std::weak_ptr<mgr::Room> room() const { return room_; }
  void room(std::weak_ptr<mgr::Room> room) { room_ = std::move(room); }

  bool disconnect() const { return should_disconnect_; }

  // connectivity
  void last_received(std::chrono::steady_clock::time_point time =
                         std::chrono::steady_clock::now());
  std::chrono::steady_clock::time_point last_received() const {
    return last_recv_;
  }
  void last_send(std::chrono::steady_clock::time_point time =
                     std::chrono::steady_clock::now());
  std::chrono::steady_clock::time_point last_send() const { return last_send_; }

  // based on last recv, may set should_disconnect_ to true
  Session_Status connection_status();

public:
  // networking

  // read incoming messages from fd
  void receive();

  // process complete messages from read_buffer_
  std::deque<mgr::In_Event> process_incoming();

  // append to write_buffer_ with this event
  void process_outgoing(mgr::Out_Event &ev);

  // send what's inside write_buffer_ (if anything is)
  void send();

public:
  // disconnecting

  // only purpose of this method is for server to be able to send this
  // (potential sequence of) message to game manager before closing connection
  // with client
  std::deque<mgr::In_Event> generate_disconnect_event();
};

} // namespace prsi::net
