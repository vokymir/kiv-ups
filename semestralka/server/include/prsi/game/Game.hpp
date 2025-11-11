#pragma once

#include "prsi/game/Player.hpp"
#include "prsi/game/Room.hpp"
#include <memory>
#include <vector>

namespace prsi::game {

class Game {
private:
  std::vector<std::unique_ptr<Room>> rooms_;
  std::vector<Player *> players_;

public:
  Game();
  ~Game();
};

} // namespace prsi::game
