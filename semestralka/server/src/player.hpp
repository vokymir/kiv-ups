#pragma once

#include <chrono>
#include <string>
namespace prsi {

// where the player is
enum Player_State {
  UNNAMED,
  LOBBY,
  ROOM,
  GAME,
  NON_EXISTING, // if the player exist on server but have no relation to any
                // socket
};

struct Player_Location {
  Player_State state_;
  int room_id_; // only valid if state==room/game
};

class Player {
public:
  Player(int fd);
  ~Player();

private:
  int fd_;
  std::string nick_;

  std::string read_buffer_;
  std::string write_buffer_;

  std::chrono::steady_clock::time_point last_recv_;
  std::chrono::steady_clock::time_point last_send_;

public:
  // read from socket into read_buffer
  void receive();
  // add something to write_buffer
  void send(std::string &msg);
  // push to socket what is in write_buffer
  void flush();

  // get/set time
  void last_received(std::chrono::steady_clock::time_point time =
                         std::chrono::steady_clock::now()) {
    last_recv_ = time;
  };
  std::chrono::steady_clock::time_point last_received() const {
    return last_recv_;
  }
  void last_send(std::chrono::steady_clock::time_point time =
                     std::chrono::steady_clock::now()) {
    last_send_ = time;
  };
  std::chrono::steady_clock::time_point last_send() const { return last_send_; }

  // get/set
  int fd() const { return fd_; }
};

} // namespace prsi
