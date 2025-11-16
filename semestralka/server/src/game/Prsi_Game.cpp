#include "game/Prsi_Game.hpp"
#include "game/Card.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>

namespace prsi::game {

Prsi_Game::Prsi_Game(std::vector<Player *> &players) : players_(players) {}

void Prsi_Game::init_create_deck() {
  deck_.reserve(ALL_RANKS.size() * ALL_SUITS.size());
  for (auto &rank : ALL_RANKS) {
    for (auto &suit : ALL_SUITS) {
      deck_.emplace_back(rank, suit);
    }
  }
}

void Prsi_Game::shuffle_deck() {
  std::shuffle(deck_.begin(), deck_.end(), random_);
}

void Prsi_Game::init_deal_cards(const int hand_size) {
  for (auto &player : players_) {
    player->hand_.reserve(static_cast<size_t>(hand_size));

    std::move(deck_.end() - hand_size, deck_.end(),
              std::back_inserter(player->hand_));
    deck_.erase(deck_.end() - hand_size, deck_.end());
  }
}

void Prsi_Game::start_game() {
  init_create_deck();
  shuffle_deck();
  init_deal_cards();

  current_player_idx_ = 0;
}

} // namespace prsi::game
