#include "room.hpp"

namespace prsi {

std::vector<std::weak_ptr<Player>> Room::players() {
  std::vector<std::weak_ptr<Player>> players;

  for (auto &p : players_) {
    players.push_back(p);
  }

  return players;
}

} // namespace prsi
