#pragma once

#include "prsi/game/Model.hpp"
#include "prsi/interfaces/ILoggable.hpp"
#include "prsi/server/Session.hpp"
#include "prsi/util/Config.hpp"
#include <map>
#include <memory>

using prsi::game::Model;
using prsi::util::Config;

namespace prsi::server {

class Server : public interfaces::ILoggable {
private:
  std::unique_ptr<Config> config_;
  std::map<int, std::unique_ptr<Session>> sessions_;
  Model model_;

public:
  Server();
  ~Server();

  void run();
};

} // namespace prsi::server
