#pragma once

#include "player.hpp"
#include "room.hpp"
#include <bits/types/wint_t.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace prsi {

class Protocol {
public:
  // WRITE
  static std::string PING() { return build_message("PING"); }

  static std::string ROOMS(std::vector<std::shared_ptr<Room>> &rs) {
    std::string body = "ROOMS " + std::to_string(rs.size());

    for (const auto &r : rs) {
      body += " " + std::to_string(r->id());
      body += " " + to_string(r->state());
    }

    return build_message(body);
  }

  // = ok messages
  static std::string OK_NAME() { return build_message("OK NAME"); }

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
};

} // namespace prsi
