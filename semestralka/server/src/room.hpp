#pragma once

#include "player.hpp"
#include <memory>
#include <vector>
namespace prsi {

enum Room_State {
  OPEN,
  PLAYING,
  FINISHED,
};

constexpr std::string to_string(Room_State s) {
  switch (s) {
  case Room_State::OPEN:
    return "OPEN";
  case Room_State::PLAYING:
    return "PLAYING";
  case Room_State::FINISHED:
    return "FINISHED";
  }
  return "UNKNOWN";
}

class Room {
private:
  int id_;
  std::vector<std::shared_ptr<Player>> players_;
  Room_State state_;

public:
  Room(int id) : id_(id) {};
  int id() const { return id_; }
  Room_State state() const { return state_; }

  std::vector<std::shared_ptr<Player>> &players() { return players_; }
  void add_player(std::shared_ptr<Player> p);
  void remove_player(std::weak_ptr<Player> p);
};

} // namespace prsi
