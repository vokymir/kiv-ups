#pragma once

#include "Player.hpp"
#include "Prsi_Game.hpp"
#include <cstddef>
#include <memory>
#include <string>
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

  void add_player(int player_id, std::string nickname);
  void remove_player(int player_id);
  bool should_close(); // if empty or game cannot continue
};

} // namespace prsi::game
