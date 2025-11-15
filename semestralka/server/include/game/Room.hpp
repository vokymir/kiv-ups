#pragma once

#include "Player.hpp"
#include "Prsi_Game.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace prsi::game {

enum class Room_State {
  ROOM_WAITING,
  ROOM_PLAYING,
  ROOM_FINISHED,
};

class Room {
public:
  int id_;
  std::vector<Player *> players_;
  std::unique_ptr<Prsi_Game> game_;
  Room_State state_;
  size_t max_players_;
};

} // namespace prsi::game
