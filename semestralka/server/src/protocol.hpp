#pragma once

#include "card.hpp"
#include "player.hpp"
#include "room.hpp"
#include "server.hpp"
#include <bits/types/wint_t.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace prsi {

class Protocol {
public:
  // WRITE
  // = control messages
  static std::string PING() { return build_message("PING"); }
  static std::string SLEEP(std::shared_ptr<Player> p) {
    std::string body = "SLEEP " + p->nick();

    return build_message(body);
  }
  static std::string DEAD(std::shared_ptr<Player> p) {
    std::string body = "DEAD " + p->nick();

    return build_message(body);
  }
  static std::string AWAKE(std::shared_ptr<Player> p) {
    std::string body = "AWAKE " + p->nick();

    return build_message(body);
  }
  static std::string STATE(Server &s, std::shared_ptr<Player> p) {
    auto loc = s.where_player(p);
    std::string body;

    auto room = loc.room_.lock();
    switch (loc.state_) {
    case Player_State::NON_EXISTING:
      body += "UNKNOWN";
      break;
    case Player_State::UNNAMED:
      body += "UNNAMED";
      break;
    case Player_State::LOBBY:
      body += "LOBBY";
      break;
    case Player_State::ROOM:
      if (!room) {
        // some garbage
        body += "BAD_STATE=ROOM_NOT_FOUND";
        break;
      }
      // just send normal room info
      return ROOM(room);
    case Player_State::GAME:
      if (!room) {
        // some garbage
        body += "BAD_STATE=ROOM_NOT_FOUND";
        break;
      }
      body += "GAME \n";
      body += strip(ROOM(room)) + "\n";
      body += strip(HAND(p)) + "\n";
      body += strip(TURN(room->current_turn()));
      break;
    }

    return build_message(body);
  }

  // = lobby messages
  static std::string ROOMS(std::vector<std::shared_ptr<Room>> &rs) {
    std::string body = "ROOMS " + std::to_string(rs.size());

    for (const auto &r : rs) {
      body += " " + std::to_string(r->id());
      body += " " + to_string(r->state());
    }

    return build_message(body);
  }

  // = room messages
  static std::string ROOM(std::shared_ptr<Room> r) {
    std::string body = "ROOM " + std::to_string(r->id()) + " ";
    body += to_string(r->state()) + " ";

    body += "PLAYERS " + std::to_string(r->players().size());
    for (const auto &p : r->players()) {
      body += " " + p->nick();
      std::string status = p->did_sleep_times() == 0 ? "AWAKE" : "SLEEP";
      body += " " + status;
    }

    return build_message(body);
  }
  static std::string JOIN(std::shared_ptr<Player> p) {
    std::string body = "JOIN " + p->nick();

    return build_message(body);
  }
  static std::string LEAVE(std::shared_ptr<Player> p) {
    std::string body = "LEAVE " + p->nick();

    return build_message(body);
  }

  // = game messages
  static std::string GAME_START() { return build_message("GAME_START"); }
  static std::string HAND(std::shared_ptr<Player> p) {
    auto &hand = p->hand();
    std::string body = "HAND " + std::to_string(hand.size());

    for (const auto &c : hand) {
      body += " " + c.to_string();
    }

    return build_message(body);
  }
  static std::string TURN(const Turn &turn) {
    std::string body = "TURN ";
    body += turn.name_ + " ";

    body += "TOP " + turn.card_.to_string();

    return build_message(body);
  }

  static std::string PLAYED(std::shared_ptr<Player> p, const Card &c) {
    std::string body = "PLAYED " + p->nick();
    body += " " + c.to_string();

    return build_message(body);
  }
  static std::string SKIP(std::shared_ptr<Player> p) {
    return build_message("SKIP " + p->nick());
  }
  static std::string DRAWED(std::shared_ptr<Player> p, int count) {
    return build_message("DRAWED " + p->nick() + " " + std::to_string(count));
  }
  static std::string CARDS(const std::vector<Card> &cards) {
    std::string body = "CARDS " + std::to_string(cards.size());

    for (const auto &c : cards) {
      body += " " + c.to_string();
    }

    return build_message(body);
  }

  static std::string WIN() { return build_message("WIN"); }

  // = ok messages
  static std::string OK_NAME() { return build_message("OK NAME"); }
  static std::string OK_JOIN_ROOM() { return build_message("OK JOIN_ROOM"); }
  static std::string OK_CREATE_ROOM() {
    return build_message("OK CREATE_ROOM");
  }
  static std::string OK_LEAVE_ROOM() { return build_message("OK LEAVE_ROOM"); }
  static std::string OK_PLAY() { return build_message("OK PLAY"); }

  // = fail messages
  static std::string FAIL_JOIN_ROOM() {
    return build_message("FAIL JOIN_ROOM");
  }
  static std::string FAIL_CREATE_ROOM() {
    return build_message("FAIL CREATE_ROOM");
  }

  // READ

  // return a message split by whitespaces into vector,
  // if no complete message, return empty vector
  // remove found messafge from mutable_string
  static std::vector<std::string> extract_message(std::string &mutable_string);

  // could the message even be validated - is long enough?
  static bool could_validate(const std::string &msg);

  // validate any string without mutating
  // does string start with magic?
  static bool valid(const std::string &msg);

private:
  // WRITE
  static inline const std::string MAGIC = "PRSI";
  static inline const std::string DELIM = "|";

  // format any message into valid protocol message
  static std::string build_message(const std::string &body) {
    // better having more white spaces than less
    // because in this protocol white spaces are ignored
    return " " + MAGIC + " " + body + " " + DELIM + "\n";
  }

  // used in STATE message
  static std::string strip(const std::string &s) {
    auto start = s.find(MAGIC);
    if (start == std::string::npos) {
      start = 0;
    } else {
      start += MAGIC.size();
    }

    auto end = s.rfind(DELIM);
    if (end == std::string::npos || end <= start)
      return {};

    return s.substr(start, end - start);
  }
};

} // namespace prsi
