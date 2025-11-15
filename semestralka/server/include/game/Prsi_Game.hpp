#pragma once

#include "Player.hpp"
#include <vector>

namespace prsi::game {

class Prsi_Game {
  std::vector<Card> deck_;
  std::vector<Card> pile_;
  std::vector<Player> players_;
  int current_player_idx_;

  bool is_valid_play(const Card &card);
  void play_card(int player_id, const Card &card);
  void draw_card(int player_id);
  void pass(int player_id);

  std::vector<Player *> get_leaderboard();
};

} // namespace prsi::game
