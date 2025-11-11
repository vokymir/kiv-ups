#pragma once

#include "Protocol.hpp"
#include "SessionManager.hpp"
#include "prsi/util/Config.hpp"
#include <memory>

using prsi::util::Config;

namespace prsi::server {

class Server {
private:
  std::unique_ptr<Config> config_;
  std::unique_ptr<SessionManager> session_manager_;
  std::unique_ptr<Protocol> protocol_;

public:
  Server();
  ~Server();

  void run();
};

} // namespace prsi::server
