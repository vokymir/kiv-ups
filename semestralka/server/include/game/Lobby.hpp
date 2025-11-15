#pragma once

#include "Room.hpp"
#include <vector>

namespace prsi::game {

class Lobby {
private:
  std::vector<Room> rooms_;

public:
  const std::vector<const Room> get_rooms();
  bool add_room();
  // but also calculate leaderboard and notify all players of it before removing
  // the room
  void remove_room(int room_id);
};

} // namespace prsi::game
