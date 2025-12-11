#pragma once

#include "player.hpp"
#include <memory>
#include <vector>
namespace prsi {

enum Room_State {
  OPEN,
  FULL,
  PLAYING,
  FINISHED,
};

class Room {
private:
  std::vector<std::shared_ptr<Player>> players_;

public:
  std::vector<std::weak_ptr<Player>> players();
  void add_player(std::shared_ptr<Player> p);
  void remove_player(std::weak_ptr<Player> p);
};

} // namespace prsi
