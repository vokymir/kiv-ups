
#include "game/Room.hpp"
#include "game/Player.hpp"
#include "util/Logger.hpp"
#include <algorithm>
#include <iterator>
namespace prsi::game {

int Room::id() const { return id_; }

std::vector<int> Room::get_player_fds() const {
  std::vector<int> fds;
  if (players_.empty()) {
    return fds;
  }
  fds.reserve(players_.size());
  std::transform(players_.begin(), players_.end(), std::back_inserter(fds),
                 [](const Player &p) { return p.id_; });
  std::sort(fds.begin(), fds.end());

  return fds;
}

bool Room::add_player(int player_id, std::string nickname) {
  if (players_.size() >= max_players_) {
    util::Logger::error("Cannot add another player to full room...");
    return false;
  } else if (state_ != Room_State::WAITING) {
    util::Logger::error("Cannot add player to ongoing game...");
    return false;
  }

  players_.emplace_back(Player{player_id, nickname});
  return true;
}

void Room::remove_player(int player_id) {
  auto it = std::find_if(
      players_.begin(), players_.end(),
      [player_id](const Player &player) { return player.id_ == player_id; });

  if (it != players_.end()) {
    players_.erase(it);
  }
}

bool Room::should_close() const {
  if (players_.empty() ||
      (state_ == Room_State::PLAYING && game_->still_playing_ <= 1)) {
    return true;
  }

  return false;
}

} // namespace prsi::game
