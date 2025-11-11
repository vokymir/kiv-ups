#pragma once

#include "prsi/game/Card.hpp"
#include <vector>

namespace prsi::game {

class Player {
private:
  std::vector<Card> hand_;

public:
  Player();
  ~Player();
};

} // namespace prsi::game
