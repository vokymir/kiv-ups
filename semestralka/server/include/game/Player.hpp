#pragma once

#include "Card.hpp"
#include <string>
#include <vector>

namespace prsi::game {

class Player {
  int id_;
  std::string name_;
  std::vector<Card> hand_;
};

} // namespace prsi::game
