#pragma once

#include "Player.hpp"
#include <random>
#include <vector>

namespace prsi::game {

class Prsi_Game {
private:
  // one true-random
  inline static std::mt19937 random_{std::random_device{}()};

public:
  std::vector<Card> deck_;
  std::vector<Card> pile_;
  std::vector<Player *> players_;
  std::vector<Player *> leaderboard_;
  int still_playing_ = -1;
  int current_player_idx_ = -1;

public:
  Prsi_Game(std::vector<Player *> &players);
  void start_game();
  bool is_valid_play(const Card &card);
  void play_card(int player_id, const Card &card);
  void draw_card(int player_id);
  void pass(int player_id);

  std::vector<Player *> get_leaderboard();

private:
  void init_create_deck();
  void shuffle_deck();
  void init_deal_cards(const int hand_size = 5);
};

} // namespace prsi::game
