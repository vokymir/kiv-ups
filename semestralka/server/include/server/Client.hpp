#pragma once

#include "game/Room.hpp"
#include <chrono>
#include <string>

namespace prsi::server {

enum class Client_State {
  CLIENT_CONNECTING,
  CLIENT_IN_LOBBY,
  CLIENT_PLAYING,
};

class Client {
public:
  int fd_;
  std::string nickname_;
  std::string read_buffer_;
  std::string write_buffer_;
  prsi::game::Room *current_room_;
  Client_State state_;
  std::chrono::steady_clock::time_point last_activity_;
};

} // namespace prsi::server
