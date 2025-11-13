#pragma once

#include "prsi/game/Card.hpp"
#include "prsi/game/Room.hpp"
#include "prsi/util/Config.hpp"
#include <vector>

namespace prsi::perspective {

struct Player_Descriptor {
  util::player_id pid_;
  int n_cards_;
};

enum Perspective_State {
  GAME_NOT_STARTED_YET,
  GAME_PLAYING,
  GAME_ENDED,
};

struct Perspective {
public:
  util::player_id player_id_;
  game::room_id room_id_;
  std::vector<Player_Descriptor> ordered_players_;
  Perspective_State state_descriptor_;
  union {
    struct {
      std::vector<game::Card> hand_;
      game::Card card_on_top_;
    } playing_;
    std::vector<util::player_id> leaderboard_;
  } state_;
};

} // namespace prsi::perspective
