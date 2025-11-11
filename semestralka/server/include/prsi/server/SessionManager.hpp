#pragma once

#include "prsi/server/Session.hpp"
#include <memory>

namespace prsi::server {

class SessionManager {
private:
  std::unique_ptr<Session> sessions_;

public:
  SessionManager();
  ~SessionManager();
};

} // namespace prsi::server
