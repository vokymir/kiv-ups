#pragma once

#include "Game.hpp"
#include "Lobby.hpp"
#include <memory>
#include <vector>

namespace prsi::game {

class GameManager {
private:
  std::unique_ptr<Lobby> lobby_;
  std::vector<std::unique_ptr<Game>> games_;

public:
  GameManager();
  ~GameManager();
};

} // namespace prsi::game
