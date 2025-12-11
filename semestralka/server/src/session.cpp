
#include "prsi/net/session.hpp"
#include "prsi/util/logger.hpp"
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
namespace prsi::net {

Session::Session(int ping_timeout_ms, int sleep_timeout_ms,
                 int death_timeout_ms)
    : ping_timeout_ms_(ping_timeout_ms), sleep_timeout_ms_(sleep_timeout_ms),
      death_timeout_ms_(death_timeout_ms) {}

Session::~Session() = default;

void Session::receive() {
  char buff[1024];
  int flags = 0;

  while (true) {
    ssize_t n = recv(fd_, buff, sizeof(buff), flags);

    if (n == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) { // no data currently avail
        return; // thats only because socket is set as nonblocking
      }

      util::Logger::error("recv() failed, will disconnected fd={}", fd_);
      should_disconnect_ = true;
      return;
    }

    if (n == 0) { // client closed connection
      should_disconnect_ = true;
      return;
    }

    read_buffer_.append(buff, n);
    last_received();
  }
}

} // namespace prsi::net
