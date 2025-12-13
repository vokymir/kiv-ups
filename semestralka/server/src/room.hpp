#pragma once

#include "player.hpp"
#include <cstddef>
#include <memory>
#include <string>
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

struct Turn {
  std::string name_;
  // TODO: top card
};

class Room {
  static int new_room_id_;

private:
  int id_ = -1;
  std::vector<std::shared_ptr<Player>> players_;
  Room_State state_ = Room_State::OPEN;

public:
  Room(int id = -1) : id_(id) {
    if (id_ == -1) {
      id_ = new_room_id_++;
    }
  };

  int id() const { return id_; }
  Room_State state() const { return state_; }
  void state(Room_State s) { state_ = s; }

  std::vector<std::shared_ptr<Player>> &players() { return players_; }

  bool should_begin_game(size_t required_players) {
    return state_ == Room_State::OPEN && (players_.size() == required_players);
  }

  // prepare game = deal cards & prepare pile/deck
  void setup_game(); // TODO:

  Turn current_turn(); // TODO:
};

} // namespace prsi
