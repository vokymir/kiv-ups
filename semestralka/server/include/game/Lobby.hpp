#pragma once

#include "Room.hpp"
#include <vector>

namespace prsi::game {

class Lobby {
public:
  std::vector<Room> rooms_;
};

} // namespace prsi::game
