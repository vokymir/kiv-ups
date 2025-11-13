#pragma once

#include "prsi/game/Card.hpp"
#include "prsi/util/Config.hpp"
#include <vector>

namespace prsi::game {

typedef int room_id;

struct Room {
public:
  room_id room_id_;
  // for how many players
  int room_size_;
  std::vector<util::player_id> players_ids_;
  std::vector<Playing_Card> cards_;
  int top_deck_pos_;

  // change deck_pos_ for all Playing_Cards_ not in hands
  // change top_deck_pos_ to reflect
  // the card on top played pile won't be shuffled
  void shuffle_deck();
};

} // namespace prsi::game
