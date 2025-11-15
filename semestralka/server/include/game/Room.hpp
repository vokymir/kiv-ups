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
private:
  int id_;
  std::vector<Player *> players_;
  std::unique_ptr<Prsi_Game> game_;
  Room_State state_;
  size_t max_players_;

public:
  int id() const;
  std::vector<int> get_player_fds() const;
  void add_player(int player_id, std::string nickname);
  void remove_player(int player_id);
  bool should_close() const; // if empty or game cannot continue
};

} // namespace prsi::game
