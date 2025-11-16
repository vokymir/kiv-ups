#pragma once

#include "Room.hpp"
#include <vector>

namespace prsi::game {

class Lobby {
private:
  std::vector<Room> rooms_;
  int get_next_room_id() const;

public:
  const std::vector<Room> &get_rooms() const;
  const Room *get_room(int room_id) const;
  // return room id on success, -1 on failure
  int add_room();
  void remove_room(int room_id);
};

} // namespace prsi::game
