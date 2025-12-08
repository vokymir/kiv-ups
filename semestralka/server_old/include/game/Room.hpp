#pragma once

#include "Player.hpp"
#include "Prsi_Game.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace prsi::game {

enum class Room_State {
  WAITING,
  PLAYING,
  FINISHED,
};

class Room {
private:
  int id_;
  std::vector<Player> players_;
  std::unique_ptr<Prsi_Game> game_;
  Room_State state_;
  size_t max_players_;
  bool updated_ = false;

public:
  Room(int id) : id_(id) {}
  ~Room() = default;
  // enable move, disable copy
  Room(const Room &) = delete;
  Room &operator=(const Room &) = delete;
  Room(Room &&) = default;
  Room &operator=(Room &&) = default;

  void is_updated();
  bool updated();
  void is_not_updated();

  int id() const;
  std::vector<int> get_player_fds() const;
  Player *get_player(int fd);
  // return false if player cannot be added
  bool add_player(int player_id, std::string nickname);
  void remove_player(int player_id);
  bool should_close() const; // if empty or game cannot continue
};

} // namespace prsi::game
