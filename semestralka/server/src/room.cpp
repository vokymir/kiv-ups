#include "room.hpp"
#include "card.hpp"
#include <array>
#include <random>
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

  shuffle_deck();
}

Card Room::deal_card() {
  if (deck_.size() > 0) {
    Card ret = deck_.front();
    deck_.pop();
    return ret;
  }

  // needs pile reshuffleing

  if (pile_.size() < 1) {
    // default return something - shouldn't happen, so have fun
    return Card('H', '7');
  }
  // save top
  Card top = pile_.front();
  pile_.pop();

  while (pile_.size() > 0) {
    deck_.emplace(pile_.front());
    pile_.pop();
  }

  // save top
  pile_.emplace(top);

  shuffle_deck();

  if (deck_.size() < 0) {
    // just another dummy card - shouldn't happen, 4fun
    return Card('Z', 'K');
  }

  // retry with shuffled deck
  return deal_card();
}

void Room::shuffle_deck() {
  // temporary move cards to somewhere 'shuffeable'
  std::vector<Card> tmp;
  tmp.reserve(deck_.size());

  while (!deck_.empty()) {
    tmp.push_back(deck_.front());
    deck_.pop();
  }

  // shuffle
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::shuffle(tmp.begin(), tmp.end(), gen);

  // move back
  for (auto &c : tmp) {
    deck_.push(c);
  }
}

} // namespace prsi
