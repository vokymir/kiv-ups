#pragma once

#include "Card.hpp"
#include <string>
#include <vector>

namespace prsi::game {

class Player {
public:
  int id_;
  std::string name_;
  std::vector<Card> hand_;

  Player(int id, std::string name) : id_(id), name_(name) {}
};

} // namespace prsi::game
