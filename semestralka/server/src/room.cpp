#include "room.hpp"
#include <array>
#include <vector>

namespace prsi {

int Room::new_room_id_ = 0;

void Room::setup_game() {
  generate_deck();

  // 1 card to have "TOP card"
  pile_.push(deal_card());

  // deal cards to players
  for (auto &p : players()) {
    auto &hand = p->hand();
    for (int i = 0; i < 4; i++) {
      hand.push_front(deal_card());
    }
  }

  // set first player
  current_player_idx_ = 0;
}

Turn Room::current_turn() {
  if (state_ != Room_State::PLAYING) {
    return {};
  }
  // current player
  Turn t;
  t.name_ = players_[current_player_idx()]->nick();

  // top card
  auto top = pile_.back();
  t.card_.rank_ = top.rank_;
  t.card_.suit_ = top.suit_;

  return t;
}

void Room::generate_deck() {
  std::array<char, 8> ranks{'7', '8', '9', '0', 'J', 'Q', 'K', 'A'};
  std::array<char, 4> suits{'Z', 'L', 'K', 'S'};

  for (const auto &r : ranks) {
    for (const auto &s : suits) {
      deck_.emplace(s, r);
    }
  }
}

} // namespace prsi
