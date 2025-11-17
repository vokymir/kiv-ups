#include "game/Prsi_Game.hpp"
#include "game/Card.hpp"
#include "game/Player.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>

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

void Prsi_Game::deal_cards(const int player_id, const int how_many) {
  // find player
  auto p_it = std::find_if(
      players_.begin(), players_.end(),
      [player_id](const Player *p) { return p->id_ == player_id; });
  if (p_it == players_.end()) {
    throw std::logic_error("Player doesn't exist");
  }
  auto *player = *p_it;

  std::move(deck_.end() - how_many, deck_.end(),
            std::back_inserter(player->hand_));
  deck_.erase(deck_.end() - how_many, deck_.end());
}

void Prsi_Game::init_deal_cards(const int hand_size) {
  for (auto &player : players_) {
    player->hand_.reserve(static_cast<size_t>(hand_size));

    deal_cards(player->id_, hand_size);
  }
}

void Prsi_Game::start_game() {
  init_create_deck();
  shuffle_deck();
  init_deal_cards();

  // have one card to start with
  top_ = deck_.front();
  top_effect_ = false;

  current_player_idx_ = 0;
}

bool Prsi_Game::is_valid_play(const Card &card) const {
  if (top_effect_) {
    if (top_.rank() == Rank::SEDM) {
      return card.rank() == Rank::SEDM;
    } else if (top_.rank() == Rank::ESO) {
      return card.rank() == Rank::ESO;
    }
  }

  if (card.rank() == Rank::MENIC) {
    return true;
  }

  return top_.suit() == card.suit() || top_.rank() == card.rank();
}

bool Prsi_Game::is_turn(int player_id) {
  return players_[static_cast<size_t>(current_player_idx_)]->id_ == player_id;
}

void Prsi_Game::play_card(int player_id, const Card &card) {
  if (!is_turn(player_id)) {
    throw std::logic_error("Cannot play outside own turn");
  }
  if (!is_valid_play(card)) {
    throw std::logic_error("Cannot play this card");
  }
  // find player
  auto p_it = std::find_if(
      players_.begin(), players_.end(),
      [player_id](const Player *p) { return p->id_ == player_id; });
  if (p_it == players_.end()) {
    throw std::logic_error("Player doesn't exist");
  }
  auto *player = *p_it;
  // find card in hand
  auto c_it = std::find_if(player->hand_.begin(), player->hand_.end(),
                           [&card](const Card &c) { return card == c; });
  if (c_it == player->hand_.end()) {
    throw std::logic_error("Player cannot play card which doesnt have");
  }
  // remove from hand
  pile_.push_back(*c_it);
  player->hand_.erase(c_it);
  top_ = card;
  if (card.rank() == Rank::SEDM || card.rank() == Rank::ESO) {
    top_effect_ = true;
    if (card.rank() == Rank::SEDM) {
      draw_on_seven_ += 2;
    }
  } else {
    top_effect_ = false;
  }

  // already won?
  if (player->hand_.empty()) {
    leaderboard_.push_back(player);
  }

  // set next player
  current_player_idx_ = get_next_player_idx();
}

void Prsi_Game::draw_card(int player_id) {
  if (!is_turn(player_id)) {
    throw std::logic_error("Cannot draw when not players turn");
  }
  if (top_effect_) {
    throw std::logic_error("Cannot draw card when some card have effect");
  }

  deal_cards(player_id, 1);
}

void Prsi_Game::pass(int player_id) {
  if (!is_turn(player_id)) {
    throw std::logic_error("Cannot pass if not their turn");
  }
  if (!top_effect_) {
    throw std::logic_error("Cannot pass if no card in effect");
  }

  if (top_.rank() == Rank::SEDM) {
    deal_cards(player_id, draw_on_seven_);
    draw_on_seven_ = 0;
  } else if (top_.rank() == Rank::ESO) {
    return;
  }

  top_effect_ = false;
}

int Prsi_Game::still_playing() const {
  int counter = 0;
  for (auto player : players_) {
    if (player == nullptr) {
      continue;
    }
    if (player->hand_.size() > 0) {
      counter++;
    }
  }
  return counter;
}

std::vector<Player *> Prsi_Game::get_leaderboard() const {
  return leaderboard_;
}

int Prsi_Game::get_next_player_idx() const {
  if (still_playing() <= 1) {
    return current_player_idx_;
  }

  int idx = current_player_idx_;
  while (static_cast<size_t>(idx) < players_.size()) {
    if (!players_[static_cast<size_t>(idx)]->hand_.empty()) {
      return idx;
    }
  }
  idx = 0;
  while (idx < current_player_idx_) {
    if (!players_[static_cast<size_t>(idx)]->hand_.empty()) {
      return idx;
    }
  }
  throw std::logic_error(
      "There must have been someone playing, but cannot have index");
}

} // namespace prsi::game
