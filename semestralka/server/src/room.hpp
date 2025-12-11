#pragma once

#include "player.hpp"
#include <vector>
namespace prsi {

enum Room_State {
  OPEN,
  FULL,
  PLAYING,
  FINISHED,
};

class Room {
  std::vector<Player> players;
};

} // namespace prsi
