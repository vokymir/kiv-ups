#pragma once

#include "game/Room.hpp"
#include "server/Message.hpp"
#include <chrono>
#include <optional>
#include <string>

namespace prsi::server {
class Server;

enum class Client_State {
  CONNECTED,
  LOBBY,
  ROOM,
  WAIT_ON_TURN,
  PLAYING,
  GAME_FINISHED,
};

enum class Client_Connection {
  OK,
  BRIEF_DISCONNECT,
  LONG_DISCONNECT,
};

class Client {
private:
  int fd_;
  std::string nickname_;
  std::string read_buffer_;
  std::string write_buffer_;
  prsi::game::Room *current_room_;
  Client_State state_;
  Client_Connection connection_;
  std::chrono::steady_clock::time_point last_received_;
  std::chrono::steady_clock::time_point last_sent_;
  std::optional<Server_Message> last_sent_msg_;

public:
  Client(int fd) : fd_(fd) {}
  ~Client() = default;

  int fd() const;

private: // should it even be there?
  void set_nickname(const std::string &nick);
  void set_current_room(game::Room *room);
  void set_state(Client_State state);

public:
  const std::string &nickname() const;

  std::string &read_buffer();
  std::string &write_buffer();

  game::Room *current_room() const;

  Client_State state() const;

  void set_connection(const Client_Connection connection);
  Client_Connection connection() const;

  void set_last_message(const Server_Message &message);
  // check which type was the last sent message
  template <typename T> bool last_sent_msg_was() const {
    return last_sent_msg_ && std::holds_alternative<T>(*last_sent_msg_);
  }

  void set_last_received(const std::chrono::steady_clock::time_point &when);
  void set_last_received_now();
  const std::chrono::steady_clock::time_point last_received() const;

  void set_last_sent(const std::chrono::steady_clock::time_point &when);
  void set_last_sent_now();
  const std::chrono::steady_clock::time_point last_sent() const;

  // process messages and if they require sending somthing back to the client or
  // broadcasting to the room, enqueue that using server
  void process_complete_messages(Server &server);

private:
  // get next message from read_buffer_ and erase it from that
  std::optional<std::string> extract_next_message();
  // switch for all client messages, if needed send server message via server
  void message_handler(Server &server, const Client_Message &msg);
  void handle_pong();
  void handle_nick(Server &server, const CM_Nick &msg);
};

} // namespace prsi::server
