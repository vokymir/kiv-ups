#pragma once

#include "prsi/game/Card.hpp"
#include <string>
#include <vector>
namespace prsi::game {

class Player {
private:
  std::string name_;
  std::vector<Card> hand_;

public:
  Player();
  ~Player();
};

} // namespace prsi::game
