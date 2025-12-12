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

class Server; // forward declare

class Player {
public:
  Player(Server &server, int socket_file_descriptor);
  ~Player();

private:
  int fd_;
  std::string nick_;

  Server &server_;
  std::string read_buffer_;
  std::string write_buffer_;

  // time of last sent ping
  std::chrono::steady_clock::time_point last_ping_;
  // time of last received pong
  std::chrono::steady_clock::time_point last_pong_;
  // how many sleep cycles were experienced without pong
  int sleep_intensity_ = 0;

public:
  // read from socket into read_buffer
  void receive();
  // add something to write_buffer
  // and try flushing the buffer
  void append_msg(const std::string &msg);
  // push to socket what is in write_buffer
  // if cannot the whole message, will set EPOLLOUT,
  // so it will be retried afterwards
  void try_flush();

  // get/set time
  void set_last_ping(std::chrono::steady_clock::time_point time =
                         std::chrono::steady_clock::now()) {
    last_ping_ = time;
  };
  std::chrono::steady_clock::time_point get_last_ping() const {
    return last_ping_;
  }
  void set_last_pong(std::chrono::steady_clock::time_point time =
                         std::chrono::steady_clock::now()) {
    last_pong_ = time;
  };
  std::chrono::steady_clock::time_point get_last_pong() const {
    return last_pong_;
  }
  int sleep_intensity() const { return sleep_intensity_; }
  void sleep_intensity(int si) { sleep_intensity_ = si; }

  // get/set
  int fd() const { return fd_; }
  const std::string &nick() const { return nick_; }
};

} // namespace prsi
