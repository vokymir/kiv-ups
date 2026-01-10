#pragma once

#include "card.hpp"
#include <chrono>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace prsi {
class Room; // forward declaring

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
  std::weak_ptr<Room> room_; // only valid if state==room/game
};

class Server; // forward declare

class Player : public std::enable_shared_from_this<Player> {
public:
  Player(Server &server, int socket_file_descriptor);
  ~Player();

private:
  int fd_;
  bool valid_fd_ = false;
  std::string nick_;

  Server &server_;
  std::string read_buffer_;
  std::string write_buffer_;

  std::list<Card> hand_;

  // time of last sent ping
  std::chrono::steady_clock::time_point last_ping_;
  // time of last received pong
  std::chrono::steady_clock::time_point last_pong_;
  // how many sleep cycles were experienced without pong
  int did_sleep_times_ = 0;

  int timer_fd_ = -1;

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
                         std::chrono::steady_clock::now());
  std::chrono::steady_clock::time_point get_last_pong() const {
    return last_pong_;
  }
  int did_sleep_times() const { return did_sleep_times_; }
  void did_sleep_times(int si) { did_sleep_times_ = si; }

  // get/set
public:
  int fd() const { return fd_; }
  void fd(int new_fd) {
    fd_ = new_fd;
    valid_fd_ = true;
  }

  bool valid_fd() const { return valid_fd_; }
  void valid_fd(bool is_valid) { valid_fd_ = is_valid; }

  int tfd() const { return timer_fd_; }
  void tfd(int new_tfd) { timer_fd_ = new_tfd; }

  const std::string &nick() const { return nick_; }
  void nick(const std::string &nick) {
    if (!nick_.empty()) {
      throw std::runtime_error("Cannot rename player.");
    }
    nick_ = nick;
  }

  std::list<Card> &hand() { return hand_; }
  bool have_card(const Card &c);
  void remove_card(const Card &c);
  // remove all cards from hand
  void clear_hand();

  // helper

  // return complete received message splitted by whitespaces or empty vector
  // remove that message from recv buffer
  // throw error if msg is buffer is invalid
  std::vector<std::string> complete_recv_msg();
};

} // namespace prsi
