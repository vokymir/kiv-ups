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
  void remove_room(int room_id);
};

} // namespace prsi::game
