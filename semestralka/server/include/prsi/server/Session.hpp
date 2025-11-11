#pragma once

#include "prsi/game/Model.hpp"
#include "prsi/interfaces/ILoggable.hpp"
#include <string>

using prsi::game::Player;

namespace prsi::server {

class Session : public interfaces::ILoggable {
private:
  std::string receiving_;
  std::string sending_;
  bool wanna_send_;

  Player *player_;

public:
  Session();
  ~Session();
};

} // namespace prsi::server
