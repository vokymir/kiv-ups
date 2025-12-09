#pragma once

#include <chrono>
#include <string>

namespace prsi::net {

// Handle message recv/send buffers, pings.
// Use protocol.
class Session {
  int fd_;
  std::string nick_;

  std::string read_buffer_;
  std::string write_buffer_;

  std::chrono::steady_clock::time_point last_recv_;
  std::chrono::steady_clock::time_point last_send_;
};

} // namespace prsi::net
