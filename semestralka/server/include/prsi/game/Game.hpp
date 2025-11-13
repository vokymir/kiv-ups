#pragma once

#include "prsi/game/Card.hpp"
#include "prsi/game/Lobby.hpp"
#include "prsi/game/Perspective.hpp"
#include "prsi/game/Room.hpp"
#include "prsi/util/Config.hpp"
#include <vector>

namespace prsi::game {

struct Game {
private:
  Lobby lobby_;
  std::vector<Room> rooms_;

public:
  // show all rooms
  const std::vector<const Room &> list_rooms();

  // player wants to join room, return true on success
  // notify all players in lobby & in this room
  bool join_room(const util::player_id pid, const room_id rid);
  // create new room & join it, or false
  // notify all players in lobby
  bool create_room(const util::player_id pid, int room_size);
  // leave any room player is in at any time
  // after calling this function player is a) in lobby b) not in a game at all
  // if the room is empty after player left, the room is destroyed
  // notify all players in lobby & this room
  void leave_room(const util::player_id pid);

  // get complete state of a room from the perspective of one player
  perspective::Perspective get_room_state(const util::player_id pid);

  // player wants to play some card, return true if possible and played, or
  // false if cannot be played
  // notify all players in room on success
  bool play_card(const util::player_id pid, const Card &card);
  // player wants to draw card, return nullptr if is invalid operation
  // notify all players in room on success
  const Card &draw_card(const util::player_id pid);
  // player a) stoji kvuli esu b) liza si karty za 7 => doesn't do anything
  // actively, just let smth happen
  // notify all players in room on success
  const std::vector<const Card &> pass(const util::player_id pid);
};

} // namespace prsi::game
