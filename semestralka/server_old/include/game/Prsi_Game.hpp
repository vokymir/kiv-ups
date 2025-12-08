#pragma once

#include "Player.hpp"
#include "game/Card.hpp"
#include <random>
#include <vector>

namespace prsi::game {

class Prsi_Game {
private:
  // one true-random
  inline static std::mt19937 random_{std::random_device{}()};

public:
  // ordering: top of deck is the last card (deck_.back())
  std::vector<Card> deck_;
  // ordering:: top of pile is the last played card (pile_.back())
  std::vector<Card> pile_;
  std::vector<Player *> players_;
  std::vector<Player *> leaderboard_;
  int current_player_idx_ = -1;
  // if the top card have effect (7, ace) only set true for next player
  bool top_effect_ = false;
  int draw_on_seven_ = 0;
  // have information on top card - if was changed by MENIC for instance, it
  // will be changed correspondingly
  Card top_{Rank::KRAL, Suit::KULE};

public:
  Prsi_Game(std::vector<Player *> &players);
  void start_game();
  bool is_turn(const int player_id);
  bool is_valid_play(const Card &card) const;
  void play_card(const int player_id, const Card &card);
  void draw_card(const int player_id);
  void pass(const int player_id);

  int still_playing() const;
  std::vector<Player *> get_leaderboard() const;

private:
  void init_create_deck();
  void shuffle_deck();
  void deal_cards(const int player_id, const int how_many);
  void init_deal_cards(const int hand_size = 5);
  // skip all players with empty hand
  int get_next_player_idx() const;
};

} // namespace prsi::game
