#pragma once

#include "prsi/server/Session.hpp"
#include "prsi/util/Config.hpp"
#include <memory>
#include <vector>

using prsi::util::Config;

namespace prsi::server {

class Server {
private:
  std::unique_ptr<Config> config_;
  std::vector<std::unique_ptr<Session>> sessions_;

public:
  Server();
  ~Server();

  void run();
};

} // namespace prsi::server
