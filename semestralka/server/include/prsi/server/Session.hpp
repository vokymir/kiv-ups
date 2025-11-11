#pragma once

#include "prsi/game/Player.hpp"
#include <memory>
#include <optional>
#include <string>

namespace prsi::server {

class Session {
private:
  std::string receiving_;
  std::string sending_;
  bool wanna_send_;

  std::optional<std::unique_ptr<Player>> player_;

public:
  Session();
  ~Session();
};

} // namespace prsi::server
